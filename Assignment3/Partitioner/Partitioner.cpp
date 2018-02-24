#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
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

    std::cout << "Grid size is " << mParsedInput.numRows << " rows x " << mParsedInput.numCols << " cols" << std::endl;
    std::cout << "Number of nodes is " << mParsedInput.numNodes << std::endl;
    std::cout << "Number of connections is " << mParsedInput.numConnections << std::endl;

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
    std::vector<unsigned int> swapCandidates;
    std::vector<posStruct_t> bestNodePositions;
    std::vector<unsigned int> partitionPopulation[2];
    unsigned int i, lockCount ;
    int currentMaxGain;

    switch (mState)
    {
        case STATE_INIT:
            mCurrentIteration = 0;
            mStartTime = clock();
            // Place the nodes at random
            netList.randomizeNodePlacement();
            // Record the partition where each node is in
            for (i = 0; i < netList.getNumNodes(); i++)
            {
                partitionPopulation[netList.getNodePartition(i)].push_back(i);
            }
            mState = STATE_PARTITIONING_START;
            break;
        case STATE_PARTITIONING_START:
            // Unlock all nodes
            netList.unlockAllNodes();
            // Make best cut size negative (indicates we haven't gotten one yet)
            mBestCutSize = -1;
            mState = STATE_PARTITIONING_FIND_SWAP_CANDIDATES;
            break;
        case STATE_PARTITIONING_FIND_SWAP_CANDIDATES:
            // Calculate total gain
            mCurrentGain = netList.calculateTotalGain();
            // Calculate the current cut size
            mCurrentCutSize = netList.calculateCurrentCutSize();
            // Check if this is better than our current cut size
            // If our best cut size is negative, then always update
            if (mCurrentCutSize < mBestCutSize || mBestCutSize < 0)
            {
                // New best cut size
                mBestCutSize = mCurrentCutSize;
                // Take note of the current node positions for this best cut size
                bestNodePositions.clear();
                for (i = 0; i < netList.getNumNodes(); i++)
                {
                    bestNodePositions.push_back(netList.getNodePosition(i));
                }
            }
            // Go through the node list and find a list of swap candidates (those with the highest gain)
            currentMaxGain = -999;
            // Keep track of how many are locked
            lockCount = 0;
            for (i = 0; i < netList.getNumNodes(); i++)
            {
                if (netList.isNodeLocked(i))
                {
                    lockCount++;
                    continue;
                }
                // Check if we've got a node with a higher gain that what we currently have
                if (netList.getNodeGain(i) > currentMaxGain)
                {
                    // Clear out swap candidates
                    swapCandidates.clear();
                    // New max gain
                    currentMaxGain = netList.getNodeGain(i);
                    // Push back the first candidate
                    swapCandidates.push_back(i);
                }
                // Check if we have a candidate with the same gain
                else if (netList.getNodeGain(i) == currentMaxGain)
                {
                    // We have another candidate
                    swapCandidates.push_back(i);
                }
            }
            if (lockCount == netList.getNumNodes())
            {
                // No more candidates! Reset the board to the best cut
                for (i = 0; i < netList.getNumNodes(); i++)
                {
                    netList.updateNodePosition(i, bestNodePositions[i]);
                }
                // Increment our iteration
                mCurrentIteration++;

                if (mCurrentIteration == MAX_ITERATIONS)
                {
                    mState = STATE_FINISHED;
                }
                else
                {
                    mState = STATE_PARTITIONING_START;
                }
            }
            // We should now have some candidates! Time to choose a node tha
            break;
        case STATE_PARTITIONING_SWAP_AND_LOCK:
            break;
        case STATE_FINISHED:
            mEndTime = clock();
            break;
        default:
            break;
    }
}

std::string Partitioner::getInfoportString()
{
    std::stringstream stringStream;

    stringStream << std::fixed << std::setprecision(3);
    stringStream << "Curr Gain:   " << std::setw(12) << mCurrentGain << "   ";
    //stringStream << "Worst HPS:   " << std::setw(12) << partitionerStruct->startingHalfPerimSum << "   ";
    //stringStream << "Current HPS: " << std::setw(12) << partitionerStruct->currentHalfPerimSum << "   ";
    //stringStream << "Improvement: " << std::setw(11) << 100.0 * static_cast<double>(difference) / static_cast<double>(partitionerStruct->startingHalfPerimSum) << "%   " << std::endl;
    //stringStream << "Start Temp:  " << std::setw(12) << partitionerStruct->startTemperature << "   ";
    //stringStream << "Decrements:  " << std::setw(12) << partitionerStruct->totalTempDecrements << "   ";
    //stringStream << "Swap:       " << std::setw(6) << partitionerStruct->currentMove << "/" << std::setw(6) << partitionerStruct->movesPerTempDec << "   ";
    //stringStream << "Acceptance:  " << std::setw(11) << 100.0 * calculateAcceptanceRate(partitionerStruct->acceptanceTracker) << "%   ";
    stringStream << std::endl;
    stringStream << "State:       ";
    switch (mState)
    {
        case STATE_INIT:
            stringStream << "Starting partitioning!";
            break;
        case STATE_PARTITIONING_START:
            stringStream << "Start partitioning...        Elapsed time: " << std::setw(10) << (clock() - mStartTime) / 1000 << "s" << std::endl;
            break;
        case STATE_PARTITIONING_FIND_SWAP_CANDIDATES:
            stringStream << "Finding swap candidates...   Elapsed time: " << std::setw(10) << (clock() - mStartTime) / 1000 << "s" << std::endl;
            break;
        case STATE_PARTITIONING_SWAP_AND_LOCK:
            stringStream << "Swap and locking...          Elapsed time: " << std::setw(10) << (clock() - mStartTime) / 1000 << "s" << std::endl;
            break;
        case STATE_FINISHED:
            stringStream << "* Finished! *  Elapsed time: " << std::setw(10) << (mEndTime - mStartTime) / 1000 << "s" << std::endl;
            break;
        default:
            break;
    }
    stringStream << "Filename:    " << mFilename;

    return stringStream.str();
}