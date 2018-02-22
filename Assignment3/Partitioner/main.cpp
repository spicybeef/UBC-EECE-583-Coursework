// C++ Includes
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <vector>

// SFML Includes
#include <SFML/Graphics.hpp>

// Program Includes
#include "Partitioner.h"
#include "NetList.h"

int main(int argc, char **argv)
{
    unsigned int i, swapCount;
	// File handling
    placer->filename = argv[1];
    //placer->filename = const_cast<char *>("..\\..\\benchmarks\\cm151a.txt");
	// Viewport size
    const sf::Vector2u viewportSize(
        static_cast<unsigned int>(WIN_VIEWPORT_WIDTH),
        static_cast<unsigned int>(WIN_VIEWPORT_HEIGHT));
	// Drawing components
    std::vector<sf::RectangleShape> backgroundGrid;
	std::vector<sf::RectangleShape> placedCells;
    std::vector<sf::Vertex> netLines;
	// Parsed input and partitioner structs
	parsedInputStruct_t *input;
	partitionerStruct_t *placer;
	// NetList object
	NetList *netList;

    // Background
    sf::RectangleShape background(sf::Vector2f(WIN_GRAPHICPORT_WIDTH, WIN_GRAPHICPORT_HEIGHT));
    background.setPosition(sf::Vector2f(0.f, 0.f));
    background.setFillColor(sf::Color(200, 200, 200, 255));
    // Log
    sf::Font font;
    sf::Text text;
    font.loadFromFile("consola.ttf");
	//font.loadFromFile("C:\\Windows\\Fonts\\consola.ttf");
    text.setFont(font);
    text.setCharacterSize(17);
    text.setFillColor(sf::Color::Green);
    text.setStyle(sf::Text::Regular);
    text.setPosition(sf::Vector2f(0.f + WIN_INFOPORT_PADDING, WIN_VIEWPORT_HEIGHT - WIN_INFOPORT_HEIGHT + WIN_INFOPORT_PADDING));
    // Separator
    sf::Vertex line[] =
    {
        sf::Vertex(sf::Vector2f(0.f, WIN_VIEWPORT_HEIGHT - WIN_INFOPORT_HEIGHT)),
        sf::Vertex(sf::Vector2f(WIN_VIEWPORT_WIDTH, WIN_VIEWPORT_HEIGHT - WIN_INFOPORT_HEIGHT))
    };

    // Filename to read in is the second argument
    std::ifstream myfile(placer->filename, std::ios::in);
    // Check if file was opened properly
    if(myfile.is_open())
    {
        std::cout << "File " << placer->filename << " opened! Here's what's in it:" << std::endl;
    }
    else
    {
        std::cout << "FATAL ERROR! File " << placer->filename << " could not be opened!" << std::endl;
        return -1;
    }

    // Parse input file
    parseInputFile(&myfile, input);

	// Instantiate the NetList
	netList = new NetList(input);

    // Get a grid, only need to do this once since it is static
	backgroundGrid = netList->generateGridGeometries();

    // Place the cells at random
	netList->randomizeNodePlacement();
    // Initialize placer state
    placer->currentState = STATE_START;
    
    // Create our render window object
    // Give it a default type (titlebar, close button)
    sf::RenderWindow window(sf::VideoMode(
        static_cast<unsigned int>(WIN_VIEWPORT_WIDTH),
        static_cast<unsigned int>(WIN_VIEWPORT_HEIGHT)),
        "Partitioner", sf::Style::Titlebar | sf::Style::Close | sf::Style::Resize);
    window.setView(calcView(window.getSize(), viewportSize));

	// Do partitioning and output results
    swapCount = 0;
    while(window.isOpen())
    { 
        // Run our partitioning
        doPartitioning(placer);
        swapCount++;
        // Only update every so often to speed up process
		/*
        if(swapCount < static_cast<unsigned int>(0.01 * static_cast<double>(placer->movesPerTempDec)))
        {
            continue;
        }
        else
        {
            swapCount = 0;
        }
		*/

        sf::Event event;
        while(window.pollEvent(event))
        {
            if(event.type == sf::Event::Closed)
                window.close();
            if(event.type == sf::Event::Resized)
                window.setView(calcView(sf::Vector2u(event.size.width, event.size.height), viewportSize));
        }

        // Get net lines
        netLines = netList->generateNetGeometries();
		// Get placed cells
		placedCells = netList->generatePlacedNodeGeometries();

		// Clear window
        window.clear();
        // Draw background
        window.draw(background);
        // Draw infoport
        text.setString(getInfoportString(placer));
        window.draw(text);
        // Draw separator
        window.draw(line, 2, sf::Lines);
        // Draw grid
        for(i = 0; i < backgroundGrid.size(); i++)
        {
            window.draw(backgroundGrid[i]);
        }

		// Draw placed cells
		for (i = 0; i < placedCells.size(); i++)
		{
			window.draw(placedCells[i]);
		}
        // Draw net lines
        window.draw(&netLines.front(), netLines.size(), sf::Lines);

        // Display window
        window.display();
    }

    return 0;
}

void doPartitioning(partitionerStruct_t *partitionerStruct)
{
    unsigned int i, randPicks[2];
    int oldHalfPerimSum, newHalfPerimSum, cost;
    double standardDev, randomDouble;
    bool acceptSwap;

    switch(partitionerStruct->currentState)
    {
        case STATE_START:
			partitionerStruct->startTime = clock();
            partitionerStruct->totalTempDecrements = 0;
            partitionerStruct->costTracker.clear();
            partitionerStruct->acceptanceTracker.clear();
            // Determine moves per temperature decrease 10 * N^(4/3)
            partitionerStruct->movesPerTempDec = static_cast<unsigned int>(10.0 * pow(static_cast<double>(placer->cells.size()), 4.0 / 3.0));
            //std::cout << "For " << placer->cells.size() << " cells, each temperature decrement will have ";
            //std::cout << partitionerStruct->movesPerTempDec << " moves" << std::endl;
            // Obtain the initial total half perimeter
            partitionerStruct->startingHalfPerimSum = calculateTotalHalfPerim(partitionerStruct->nets);
            // Determine the initial temperature
            // Perform 50 swaps
            for(i = 0; i < 50; i++)
            {
                // Record initial total half perimeter
                oldHalfPerimSum = calculateTotalHalfPerim(partitionerStruct->nets);
                // Pick two cells at random
                randPicks[0] = getRandomInt(static_cast<unsigned int>(partitionerStruct->cells.size()));
                randPicks[1] = getRandomInt(static_cast<unsigned int>(partitionerStruct->cells.size()));
                // Swap them
                swapCells(&partitionerStruct->cells[randPicks[0]], &partitionerStruct->cells[randPicks[1]], partitionerStruct);
                // Record the new total half perimeter
                newHalfPerimSum = calculateTotalHalfPerim(partitionerStruct->nets);
                // Push back cost
                //std::cout << "Cost of swap " << i << " was " << newHalfPerimSum - oldHalfPerimSum << std::endl;
                partitionerStruct->costTracker.push_back(newHalfPerimSum - oldHalfPerimSum);
            }
            // Calculate the standard deviation, this is used for the initial temperature
            standardDev = calculateStandardDeviation(partitionerStruct->costTracker);

            //std::cout << "Standard deviation is " << standardDev << std::endl;
            partitionerStruct->startTemperature = standardDev * START_TEMP_STD_MULT;
            partitionerStruct->currentTemperature = standardDev * START_TEMP_STD_MULT;
            //std::cout << "Starting temperature is " << partitionerStruct->currentTemperature;
            
            // Reset cost tracker
            partitionerStruct->costTracker.clear();
            // Start annealin'
            partitionerStruct->currentState = STATE_PARTITIONING;
            break;
        case STATE_PARTITIONING:
            // We are annealing
            // Initially we don't accept the swap
            acceptSwap = false;
            // Calculate the current total half perimeter
            placer->currentHalfPerimSum = calculateTotalHalfPerim(placer->nets);
            // Check if we're done for this temperature
            if(partitionerStruct->currentMove == partitionerStruct->movesPerTempDec)
            {
                // Restart our move count
                partitionerStruct->currentMove = 0;
                // Determine the standard deviation
                standardDev = calculateStandardDeviation(partitionerStruct->costTracker);
                // Calculate the new temperature
                partitionerStruct->currentTemperature = calculateNewTemp(partitionerStruct->currentTemperature, standardDev, TEMP_DECREASE_EXP);
                // Increment the temperature
                partitionerStruct->totalTempDecrements++;
                // Reset the cost tracker
                partitionerStruct->costTracker.clear();
				// Check if we haven't accepted anything this past temperature decrement
				if (calculateAcceptanceRate(partitionerStruct->acceptanceTracker) <= ACCEPTANCE_RATE_CUTOFF)
				{
					// We're done!
					partitionerStruct->currentState = STATE_FINISHED;
					// Record finish time
					partitionerStruct->endTime = clock();
				}
				else
				{
					// Reset the acceptance tracker
					partitionerStruct->acceptanceTracker.clear();
				}
            }
            // We still have swaps to do
            else
            {
                // Record initial total half perimeter
                oldHalfPerimSum = calculateTotalHalfPerim(partitionerStruct->nets);
                // Pick two cells at random
                randPicks[0] = getRandomInt(static_cast<unsigned int>(partitionerStruct->cells.size()));
                randPicks[1] = getRandomInt(static_cast<unsigned int>(partitionerStruct->cells.size()));
                // Swap them
                swapCells(&partitionerStruct->cells[randPicks[0]], &partitionerStruct->cells[randPicks[1]], partitionerStruct);
                // Record the new total half perimeter
                newHalfPerimSum = calculateTotalHalfPerim(partitionerStruct->nets);
				// If it's worse than our current worse, store it
				if (static_cast<unsigned int>(newHalfPerimSum) > partitionerStruct->startingHalfPerimSum)
				{
					partitionerStruct->startingHalfPerimSum = newHalfPerimSum;
				}

                // calculate cost
                cost = newHalfPerimSum - oldHalfPerimSum;

                // If this was a bad move, if so, check if we are going to accept it
                if(newHalfPerimSum >= oldHalfPerimSum)
                {
                    randomDouble = getRandomDouble();
					//std::cout << "Random double: " << randomDouble << " ";
                    // Check if we accept swap
					//std::cout << "Cost: " << static_cast<double>(cost) << " ";
					//std::cout << "Swap exp: " << exp(-1.0 * static_cast<double>(cost) / partitionerStruct->currentTemperature) << std::endl;
					if (cost == 0)
					{
						// No point in doing this
						// Revert cell swap
						swapCells(&partitionerStruct->cells[randPicks[0]], &partitionerStruct->cells[randPicks[1]], partitionerStruct);
					}
                    else if(randomDouble < exp(-1.0 * static_cast<double>(cost) / partitionerStruct->currentTemperature))
                    {
                        // Accept the swap!
                        acceptSwap = true;
                    }
                    else
                    {
                        // Not going to happen
                        // Revert cell swap
                        swapCells(&partitionerStruct->cells[randPicks[0]], &partitionerStruct->cells[randPicks[1]], partitionerStruct);
                    }
                }
                else
                {
                    // Accept the swap!
                    acceptSwap = true;
                }

                if(acceptSwap)
                {
                    // Update the cells' net color
                    updateNetColor(partitionerStruct->cells[randPicks[0]]);
                    updateNetColor(partitionerStruct->cells[randPicks[1]]);
                    
                    // Push back acceptance
                    partitionerStruct->acceptanceTracker.push_back(true);
                    // Push back cost
                    //std::cout << "Cost of swap " << i << " was " << newHalfPerimSum - oldHalfPerimSum << std::endl;
                    partitionerStruct->costTracker.push_back(cost);
                }
                else
                {
                    // Push back non-acceptance
                    partitionerStruct->acceptanceTracker.push_back(false);
                }

                // Next move
                partitionerStruct->currentMove++;
            }
            break;
        case STATE_FINISHED:
            break;
        default:
            break;
    }
}

sf::View calcView(const sf::Vector2u &windowSize, const sf::Vector2u &viewportSize)
{
    sf::FloatRect viewport(0.f, 0.f, 1.f, 1.f);
    float viewportWidth = static_cast<float>(viewportSize.x);
    float viewportHeight = static_cast<float>(viewportSize.y);
    float screenwidth = windowSize.x / static_cast<float>(viewportSize.x);
    float screenheight = windowSize.y / static_cast<float>(viewportSize.y);

    if(screenwidth > screenheight)
    {
        viewport.width = screenheight / screenwidth;
        viewport.left = (1.f - viewport.width) / 2.f;
    }
    else if(screenwidth < screenheight)
    {
        viewport.height = screenwidth / screenheight;
        viewport.top = (1.f - viewport.height) / 2.f;
    }

    sf::View view(sf::FloatRect(0.f, 0.f, viewportWidth, viewportHeight));
    view.setViewport(viewport);

    return view;
}

bool parseInputFile(std::ifstream *inputFile, parsedInputStruct_t *inputStruct)
{
    unsigned int i, j, numNodes;
    std::string line;
    std::vector<std::string> stringVec;

    // 1. Get number of cells, number of nets, and grid size
    std::getline(*inputFile, line);
    stringVec = splitString(line, ' ');
    inputStruct->numNodes = stoi(stringVec[0]);
    inputStruct->numConnections = stoi(stringVec[1]);
    inputStruct->numRows = stoi(stringVec[2]);
    inputStruct->numCols = stoi(stringVec[3]);

    std::cout << "Grid size is " << inputStruct->numRows << " rows x " << inputStruct->numCols << " cols" << std::endl;
    std::cout << "Number of nodes is " << inputStruct->numNodes << std::endl;
    std::cout << "Number of connections is " << inputStruct->numConnections << std::endl;

    // 2. Get all connections
    for(i = 0; i < inputStruct->numConnections; i++)
    {
        std::getline(*inputFile, line);
        stringVec = splitString(line, ' ');
        // Get number of nodes for this net
        numNodes = stoi(stringVec[0]);
        // Now get all nodes for this net
        // Push back a new vector for this
        inputStruct->nets.push_back(std::vector<unsigned int>());
        for(j = 0; j < numNodes; j++)
        {
            inputStruct->nets[i].push_back(stoi(stringVec[j + 1]));
        }
    }


    return true;
}

std::string getInfoportString(partitionerStruct_t *partitionerStruct)
{
    std::stringstream stringStream;
    int difference;
	/*
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
    stringStream << "Filename:    " << placer->filename;

    return stringStream.str();
}
