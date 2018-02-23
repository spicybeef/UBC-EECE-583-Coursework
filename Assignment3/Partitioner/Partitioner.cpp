#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <vector>

#include "Partitioner.h"
#include "Util.h"

Partitioner::Partitioner()
{
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

void Partitioner::doPartitioning()
{
    // unsigned int i, randPicks[2];
    // int oldHalfPerimSum, newHalfPerimSum, cost;
    // double standardDev, randomDouble;
    // bool acceptSwap;

    // switch(partitionerStruct->currentState)
    // {
    //     case STATE_START:
    //partitionerStruct->startTime = clock();
    //         partitionerStruct->totalTempDecrements = 0;
    //         partitionerStruct->costTracker.clear();
    //         partitionerStruct->acceptanceTracker.clear();
    //         // Determine moves per temperature decrease 10 * N^(4/3)
    //         partitionerStruct->movesPerTempDec = static_cast<unsigned int>(10.0 * pow(static_cast<double>(placer->cells.size()), 4.0 / 3.0));
    //         //std::cout << "For " << placer->cells.size() << " cells, each temperature decrement will have ";
    //         //std::cout << partitionerStruct->movesPerTempDec << " moves" << std::endl;
    //         // Obtain the initial total half perimeter
    //         partitionerStruct->startingHalfPerimSum = calculateTotalHalfPerim(partitionerStruct->nets);
    //         // Determine the initial temperature
    //         // Perform 50 swaps
    //         for(i = 0; i < 50; i++)
    //         {
    //             // Record initial total half perimeter
    //             oldHalfPerimSum = calculateTotalHalfPerim(partitionerStruct->nets);
    //             // Pick two cells at random
    //             randPicks[0] = getRandomInt(static_cast<unsigned int>(partitionerStruct->cells.size()));
    //             randPicks[1] = getRandomInt(static_cast<unsigned int>(partitionerStruct->cells.size()));
    //             // Swap them
    //             swapCells(&partitionerStruct->cells[randPicks[0]], &partitionerStruct->cells[randPicks[1]], partitionerStruct);
    //             // Record the new total half perimeter
    //             newHalfPerimSum = calculateTotalHalfPerim(partitionerStruct->nets);
    //             // Push back cost
    //             //std::cout << "Cost of swap " << i << " was " << newHalfPerimSum - oldHalfPerimSum << std::endl;
    //             partitionerStruct->costTracker.push_back(newHalfPerimSum - oldHalfPerimSum);
    //         }
    //         // Calculate the standard deviation, this is used for the initial temperature
    //         standardDev = calculateStandardDeviation(partitionerStruct->costTracker);

    //         //std::cout << "Standard deviation is " << standardDev << std::endl;
    //         partitionerStruct->startTemperature = standardDev * START_TEMP_STD_MULT;
    //         partitionerStruct->currentTemperature = standardDev * START_TEMP_STD_MULT;
    //         //std::cout << "Starting temperature is " << partitionerStruct->currentTemperature;
    //         
    //         // Reset cost tracker
    //         partitionerStruct->costTracker.clear();
    //         // Start annealin'
    //         partitionerStruct->currentState = STATE_PARTITIONING;
    //         break;
    //     case STATE_PARTITIONING:
    //         // We are annealing
    //         // Initially we don't accept the swap
    //         acceptSwap = false;
    //         // Calculate the current total half perimeter
    //         placer->currentHalfPerimSum = calculateTotalHalfPerim(placer->nets);
    //         // Check if we're done for this temperature
    //         if(partitionerStruct->currentMove == partitionerStruct->movesPerTempDec)
    //         {
    //             // Restart our move count
    //             partitionerStruct->currentMove = 0;
    //             // Determine the standard deviation
    //             standardDev = calculateStandardDeviation(partitionerStruct->costTracker);
    //             // Calculate the new temperature
    //             partitionerStruct->currentTemperature = calculateNewTemp(partitionerStruct->currentTemperature, standardDev, TEMP_DECREASE_EXP);
    //             // Increment the temperature
    //             partitionerStruct->totalTempDecrements++;
    //             // Reset the cost tracker
    //             partitionerStruct->costTracker.clear();
    //  // Check if we haven't accepted anything this past temperature decrement
    //  if (calculateAcceptanceRate(partitionerStruct->acceptanceTracker) <= ACCEPTANCE_RATE_CUTOFF)
    //  {
    //      // We're done!
    //      partitionerStruct->currentState = STATE_FINISHED;
    //      // Record finish time
    //      partitionerStruct->endTime = clock();
    //  }
    //  else
    //  {
    //      // Reset the acceptance tracker
    //      partitionerStruct->acceptanceTracker.clear();
    //  }
    //         }
    //         // We still have swaps to do
    //         else
    //         {
    //             // Record initial total half perimeter
    //             oldHalfPerimSum = calculateTotalHalfPerim(partitionerStruct->nets);
    //             // Pick two cells at random
    //             randPicks[0] = getRandomInt(static_cast<unsigned int>(partitionerStruct->cells.size()));
    //             randPicks[1] = getRandomInt(static_cast<unsigned int>(partitionerStruct->cells.size()));
    //             // Swap them
    //             swapCells(&partitionerStruct->cells[randPicks[0]], &partitionerStruct->cells[randPicks[1]], partitionerStruct);
    //             // Record the new total half perimeter
    //             newHalfPerimSum = calculateTotalHalfPerim(partitionerStruct->nets);
    //  // If it's worse than our current worse, store it
    //  if (static_cast<unsigned int>(newHalfPerimSum) > partitionerStruct->startingHalfPerimSum)
    //  {
    //      partitionerStruct->startingHalfPerimSum = newHalfPerimSum;
    //  }

    //             // calculate cost
    //             cost = newHalfPerimSum - oldHalfPerimSum;

    //             // If this was a bad move, if so, check if we are going to accept it
    //             if(newHalfPerimSum >= oldHalfPerimSum)
    //             {
    //                 randomDouble = getRandomDouble();
    //      //std::cout << "Random double: " << randomDouble << " ";
    //                 // Check if we accept swap
    //      //std::cout << "Cost: " << static_cast<double>(cost) << " ";
    //      //std::cout << "Swap exp: " << exp(-1.0 * static_cast<double>(cost) / partitionerStruct->currentTemperature) << std::endl;
    //      if (cost == 0)
    //      {
    //          // No point in doing this
    //          // Revert cell swap
    //          swapCells(&partitionerStruct->cells[randPicks[0]], &partitionerStruct->cells[randPicks[1]], partitionerStruct);
    //      }
    //                 else if(randomDouble < exp(-1.0 * static_cast<double>(cost) / partitionerStruct->currentTemperature))
    //                 {
    //                     // Accept the swap!
    //                     acceptSwap = true;
    //                 }
    //                 else
    //                 {
    //                     // Not going to happen
    //                     // Revert cell swap
    //                     swapCells(&partitionerStruct->cells[randPicks[0]], &partitionerStruct->cells[randPicks[1]], partitionerStruct);
    //                 }
    //             }
    //             else
    //             {
    //                 // Accept the swap!
    //                 acceptSwap = true;
    //             }

    //             if(acceptSwap)
    //             {
    //                 // Update the cells' net color
    //                 updateNetColor(partitionerStruct->cells[randPicks[0]]);
    //                 updateNetColor(partitionerStruct->cells[randPicks[1]]);
    //                 
    //                 // Push back acceptance
    //                 partitionerStruct->acceptanceTracker.push_back(true);
    //                 // Push back cost
    //                 //std::cout << "Cost of swap " << i << " was " << newHalfPerimSum - oldHalfPerimSum << std::endl;
    //                 partitionerStruct->costTracker.push_back(cost);
    //             }
    //             else
    //             {
    //                 // Push back non-acceptance
    //                 partitionerStruct->acceptanceTracker.push_back(false);
    //             }

    //             // Next move
    //             partitionerStruct->currentMove++;
    //         }
    //         break;
    //     case STATE_FINISHED:
    //         break;
    //     default:
    //         break;
    // }
}

std::string Partitioner::getInfoportString()
{
    std::stringstream stringStream;
    /*
    int difference;

    difference = static_cast<int>(partitionerStruct->startingHalfPerimSum) - static_cast<int>(partitionerStruct->currentHalfPerimSum);

    stringStream << std::fixed << std::setprecision(3);
    stringStream << "Curr Temp:   " << std::setw(12) << partitionerStruct->currentTemperature << "   ";
    stringStream << "Worst HPS:   " << std::setw(12) << partitionerStruct->startingHalfPerimSum << "   ";
    stringStream << "Current HPS: " << std::setw(12) << partitionerStruct->currentHalfPerimSum << "   ";
    stringStream << "Improvement: " << std::setw(11) << 100.0 * static_cast<double>(difference) / static_cast<double>(partitionerStruct->startingHalfPerimSum) << "%   " << std::endl;
    stringStream << "Start Temp:  " << std::setw(12) << partitionerStruct->startTemperature << "   ";
    stringStream << "Decrements:  " << std::setw(12) << partitionerStruct->totalTempDecrements << "   ";
    stringStream << "Swap:       " << std::setw(6) << partitionerStruct->currentMove << "/" << std::setw(6) << partitionerStruct->movesPerTempDec << "   ";
    stringStream << "Acceptance:  " << std::setw(11) << 100.0 * calculateAcceptanceRate(partitionerStruct->acceptanceTracker) << "%   ";
    stringStream << std::endl;
    stringStream << "State:       ";
    switch(partitionerStruct->currentState)
    {
    case STATE_START:
    stringStream << "Starting placer!";
    break;
    case STATE_PARTITIONING:
    stringStream << "Annealing...   Elapsed time: " << std::setw(10) << (clock() - partitionerStruct->startTime) / 1000 << "s" << std::endl;
    break;
    case STATE_FINISHED:
    stringStream << "* Finished! *  Elapsed time: " << std::setw(10) << (partitionerStruct->endTime - partitionerStruct->startTime) / 1000 << "s" << std::endl;
    break;
    default:
    break;
    }
    */
    //stringStream << "Filename:    " << placer->filename;

    return stringStream.str();
}