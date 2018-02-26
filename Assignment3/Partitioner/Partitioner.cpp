#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "Partitioner.h"
#include "Util.h"

Partitioner::Partitioner()
{
    mState = STATE_INIT;
}


Partitioner::~Partitioner()
{
}

bool Partitioner::parseInputFile()
{
    unsigned int i, j, numNodes;
    std::string line;
    std::vector<std::string> stringVec;

    std::ifstream inputFile(mFilename, std::ios::in);
    // Check if file was opened properly
    if (inputFile.is_open())
    {
        std::cout << "File " << mFilename << " opened! Here's what's in it:" << std::endl;
    }
    else
    {
        std::cout << "FATAL ERROR! File " << mFilename << " could not be opened!" << std::endl;
        return false;
    }

    // 1. Get number of cells, number of nets, and grid size
    std::getline(inputFile, line);
    stringVec = splitString(line, ' ');
    mParsedInput.numNodes = stoi(stringVec[0]);
    mParsedInput.numConnections = stoi(stringVec[1]);
    mParsedInput.numRows = stoi(stringVec[2]);
    mParsedInput.numCols = stoi(stringVec[3]);

    //std::cout << "Grid size is " << mParsedInput.numRows << " rows x " << mParsedInput.numCols << " cols" << std::endl;
    //std::cout << "Number of nodes is " << mParsedInput.numNodes << std::endl;
    //std::cout << "Number of connections is " << mParsedInput.numConnections << std::endl;

    // 2. Get all connections
    for (i = 0; i < mParsedInput.numConnections; i++)
    {
        std::getline(inputFile, line);
        stringVec = splitString(line, ' ');
        // Get number of nodes for this net
        numNodes = stoi(stringVec[0]);
        // Now get all nodes for this net
        // Push back a new vector for this
        mParsedInput.nets.push_back(std::vector<unsigned int>());
        for (j = 0; j < numNodes; j++)
        {
            mParsedInput.nets[i].push_back(stoi(stringVec[j + 1]));
        }
    }

    return true;
}

parsedInputStruct_t Partitioner::getParsedInput()
{
    return mParsedInput;
}

void Partitioner::doPartitioning(NetList &netList)
{
    unsigned int i, lockCount, candidateIndex, candidateSwapIndex, candidateNodeId;
    int currentMaxGain, candidateGain;

    switch (mState)
    {
        case STATE_INIT:
            // Initialize iteration counter
            mCurrentIteration = 0;

            // Get start time
            mStartTime = clock();
            
            // Place the nodes at random
            netList.randomizeNodePlacement();

            // Calculate the starting cut size
            mStartCutSize = netList.calculateCurrentCutSize();

            // Update partition lists
            updatePartitionLists(netList);
            // Determine where we are swapping from
            determineSwap();

            // By this point, we have shuffled the nodes onto the grid
            // We've recorded the nodes into each partition vector
            // And we've determined which partition to start swapping from
            mFirstTime = true;

            // Start partitioning
            mState = STATE_PARTITIONING_START;

            break;

        case STATE_PARTITIONING_START:

            // Update partition lists
            updatePartitionLists(netList);

            // Unlock all nodes
            netList.unlockAllNodes();

            // Calculate the current cut size
            mCurrentCutSize = netList.calculateCurrentCutSize();
            // Make best cut the current cut size
            mBestCutSize = mCurrentCutSize;

            // Find some swap candidates
            mState = STATE_PARTITIONING_FIND_SWAP_CANDIDATES;

            break;

        case STATE_PARTITIONING_FIND_SWAP_CANDIDATES:

            // Update partition lists
            updatePartitionLists(netList);
            // Determine swap
            determineSwap();

            // Calculate all node gains
            netList.updateAllNodeGains();

            // Calculate the current cut size
            mCurrentCutSize = netList.calculateCurrentCutSize();
            // Check if this is better than our current cut size (or we've done this for the first time)
            if (mCurrentCutSize < mBestCutSize || mFirstTime)
            {
                // New best cut size
                mBestCutSize = mCurrentCutSize;
                // Take note of the current node positions for this best cut size
                mBestNodePositions.clear();
                for (i = 0; i < netList.getNumNodes(); i++)
                {
                    mBestNodePositions.push_back(netList.getNodePosition(i));
                }
                // No longer our first time
                mFirstTime = false;
            }

            // Go through the node list and find a list of swap candidates (those with the highest gain)
            currentMaxGain = -999;
            // Keep track of how many are locked
            lockCount = 0;
            for (i = 0; i < mPartitionNodeList[mCurrentPartition].size(); i++)
            {
                // Go to the next one if it's locked
                if (netList.isNodeLocked(mPartitionNodeList[mCurrentPartition][i]))
                {
                    lockCount++;
                    continue;
                }

                // New candidate
                candidateIndex = i;
                candidateGain = netList.getNodeGain(mPartitionNodeList[mCurrentPartition][candidateIndex]);

                // Check if it has higher gain that what we currently have
                if (candidateGain > currentMaxGain)
                {
                    // We have a new candidate!
                    // New max gain
                    currentMaxGain = candidateGain;
                    // Clear out the old swap candidates
                    mSwapCandidates.clear();
                    // Push back the first candidate
                    mSwapCandidates.push_back(candidateIndex);
                }
                // Check if we have a candidate with the same gain
                else if (candidateGain == currentMaxGain)
                {
                    // We have another candidate
                    mSwapCandidates.push_back(candidateIndex);
                }
            }

            // We should now have some candidates now
            mState = STATE_PARTITIONING_SWAP_AND_LOCK;

            // Check if the entire partition node list is locked
            if (lockCount == mPartitionNodeList[mCurrentPartition].size())
            {
                // Check if we've reached our max iterations
                if (mCurrentIteration == MAX_ITERATIONS)
                {
                    // Record end time and finish
                    mEndTime = clock();
                    // Go to finishing state
                    mState = STATE_FINISHED;
                }
                else
                {
                    // No more candidates! Reset the board to the best cut
                    for (i = 0; i < netList.getNumNodes(); i++)
                    {
                        netList.updateNodePosition(i, mBestNodePositions[i]);
                    }
                    // Increment our iteration
                    mCurrentIteration++;
                    // Go back for another iteration
                    mState = STATE_PARTITIONING_START;
                }
            }

            break;

        case STATE_PARTITIONING_SWAP_AND_LOCK:

            // Choose a candidate at random
            candidateSwapIndex = getRandomInt(static_cast<unsigned int>(mSwapCandidates.size()));
            candidateNodeId = mPartitionNodeList[mCurrentPartition][mSwapCandidates[candidateSwapIndex]];

            // Clear the swap candidates
            mSwapCandidates.clear();

            // Swap the node
            netList.swapNodePartition(candidateNodeId);

            // Lock it
            netList.lockNode(candidateNodeId);

            // Calculate the new cut size
            mCurrentCutSize = netList.calculateCurrentCutSize();

            // Find some more candidates
            mState = STATE_PARTITIONING_FIND_SWAP_CANDIDATES;

            break;

        case STATE_FINISHED:

            // Calculate the current cut size
            mCurrentCutSize = netList.calculateCurrentCutSize();

            // Update partition list
            updatePartitionLists(netList);

            // Unlock all nodes
            netList.unlockAllNodes();

            break;
        default:
            break;
    }
}

std::string Partitioner::getInfoportString()
{
    std::stringstream stringStream;

    stringStream << std::fixed << std::setprecision(3);
    stringStream << "Part Div:    " << std::setw(8) << mPartitionNodeList[0].size() << "/" << std::setw(3) << mPartitionNodeList[1].size() << "   ";
    stringStream << "Start Cut:   " << std::setw(12) << mStartCutSize << "   ";
    stringStream << "Best Cut:    " << std::setw(12) << mBestCutSize << "   ";
    stringStream << "Current Cut: " << std::setw(12) << mCurrentCutSize << "   ";
    stringStream << std::endl << std::endl;
    stringStream << "State:       ";
    switch (mState)
    {
        case STATE_INIT:
            stringStream << "Starting partitioning!";
            break;
        case STATE_PARTITIONING_START:
            stringStream << "Start partitioning...        Elapsed time: " << std::setw(10) << clock() - mStartTime << " ms" << std::endl;
            break;
        case STATE_PARTITIONING_FIND_SWAP_CANDIDATES:
            stringStream << "Finding swap candidates...   Elapsed time: " << std::setw(10) << clock() - mStartTime << " ms" << std::endl;
            break;
        case STATE_PARTITIONING_SWAP_AND_LOCK:
            stringStream << "Swap and locking...          Elapsed time: " << std::setw(10) << clock() - mStartTime << " ms" << std::endl;
            break;
        case STATE_FINISHED:
            stringStream << "* Finished! *  Elapsed time: " << std::setw(10) << mEndTime - mStartTime << " ms    with final cut size of: " << mCurrentCutSize << std::endl;
            break;
        default:
            break;
    }
    stringStream << "Filename:    " << mFilename;

    return stringStream.str();
}

void Partitioner::updatePartitionLists(NetList &netList)
{
    unsigned int i;

    // Clear out the partition lists
    mPartitionNodeList[0].clear();
    mPartitionNodeList[1].clear();
    // Record the partition where each node is in
    for (i = 0; i < netList.getNumNodes(); i++)
    {
        mPartitionNodeList[netList.getNodePartition(i)].push_back(i);
    }
}

void Partitioner::determineSwap()
{
    // Determine which partition we are swapping from
    // If size of 0 is greater than 1, then swap from 0
    if (mPartitionNodeList[0].size() > mPartitionNodeList[1].size())
    {
        mCurrentPartition = 0;
    }
    // Otherwise size of 1 is greater than 0 or equal to zero, swap from 1
    else
    {
        mCurrentPartition = 1;
    }
}

state_e Partitioner::getState()
{
    return mState;
}

void Partitioner::setState(state_e state)
{
    mState = state;
}
