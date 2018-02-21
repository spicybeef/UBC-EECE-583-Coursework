// C++ Includes
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <ctime>

// SFML Includes
#include <SFML/Graphics.hpp>

// Program Includes
#include "Partitioner.h"

parsedInputStruct_t *input = new parsedInputStruct_t();
partitionerStruct_t *placer = new partitionerStruct_t();

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

    // Seed our randomizer with the current time
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

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

    // Get a grid, only need to do this once since it is static
    backgroundGrid = generateGridGeometries(input, placer);

	// Push back enough nodes for the nets
	generateNodes(input->numCols * input->numRows, input->numNodes, placer);
	// Connect them via their nets
	generateNodeConnections(input, placer);

    // Initialize the grid model
    placer->grid = initializeGridModel(input->numCols, input->numRows);
    // Place the cells at random
    generateNodePlacement(input->numCols, input->numRows, placer->cellProperties);
    // Give each cell a pointer to their on net
    initializeNodeNet(placer->nets);
    // Set all nets to their starting color
    initializeNetColors(placer->nets, input->numCols, input->numRows);

    // Initialize placer state
    placer->currentState = STATE_START;
    
    // Create our render window object
    // Give it a default type (titlebar, close button)
    sf::RenderWindow window(sf::VideoMode(
        static_cast<unsigned int>(WIN_VIEWPORT_WIDTH),
        static_cast<unsigned int>(WIN_VIEWPORT_HEIGHT)),
        "Simulated Annealing Placer", sf::Style::Titlebar | sf::Style::Close | sf::Style::Resize);
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
        netLines = generateNetGeometries(placer->nets);
		// Get placed cells
		placedCells = generatePlacedNodeGeometries(placer->nodes, placer->cellProperties.maximizedDim, placer->cellProperties.cellSize, placer->cellProperties.cellOffset, placer->cellProperties.cellOppositeOffset, placer->currentState);

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

std::vector<std::string> splitString(std::string inString, char delimiter)
{
    std::vector<std::string> internal;
    std::stringstream ss(inString); // Turn the string into a stream.
    std::string temp;

    while(std::getline(ss, temp, delimiter))
    {
        internal.push_back(temp);
    }

    return internal;
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

int getRandomInt(int i)
{
    return std::rand() % i;
}

double getRandomDouble(void)
{
    return static_cast<double>(std::rand()) / static_cast<double>(RAND_MAX);
}

drawPosStruct_t getGridCellCoordinate(cellPropertiesStruct_t cellProperties, unsigned int col, unsigned int row)
{
	drawPosStruct_t drawPos;

	// Depending on our current maximized direction, return the drawing position for the selected column and row
	if (cellProperties.maximizedDim == DIM_VERTICAL)
	{
		drawPos.x = col * cellProperties.cellSize + cellProperties.cellOppositeOffset;
		drawPos.y = row * cellProperties.cellSize + cellProperties.cellOffset;
	}
	else
	{
		drawPos.x = col * cellProperties.cellSize + cellProperties.cellOffset;
		drawPos.y = row * cellProperties.cellSize + cellProperties.cellOppositeOffset;
	}

	return drawPos;
}

void updateNodePosition(cellPropertiesStruct_t cellProperties, nodeStruct_t &node, std::vector<std::vector<nodeStruct_t*>> &grid, unsigned int col, unsigned int row)
{
	// Update the cell's grid position
	node.pos.col = col;
	node.pos.row = row;

	// Update the cell's drawing position
	node.drawPos = getGridCellCoordinate(cellProperties, col, row);

	// Add the cell to the grid
	grid[col][row] = &node;
}

void generateNodeConnections(parsedInputStruct_t *inputStruct, partitionerStruct_t *partitionerStruct)
{
    unsigned int i, j;
    nodeStruct_t *cellPointer;

	// Clear any existing nets
    partitionerStruct->nets.clear();

	// Go through the parsed input, and add each net
    for(i = 0; i < inputStruct->nets.size(); i++)
    {
        // Push a fresh net to the placer
        partitionerStruct->nets.push_back(netStruct_t());

        for(j = 0; j < inputStruct->nets[i].size(); j++)
        {
            // Give the net its node references
            cellPointer = &partitionerStruct->nodes[inputStruct->nets[i][j]];
            partitionerStruct->nets.back().connections.push_back(cellPointer);
        }
    }
}

std::vector<std::vector<nodeStruct_t*>> initializeGridModel(unsigned int numCols, unsigned int numRows)
{
    unsigned int i, j;
	std::vector<std::vector<nodeStruct_t*>> grid;

    // Generate the grid model
    for(i = 0; i < numCols; i++)
    {
        grid.push_back(std::vector<nodeStruct_t*>());
        for(j = 0; j < numRows; j++)
        {
            grid[i].push_back(NULL);
        }
    }

	return grid;
}

void generateNodes(unsigned int totalCells, unsigned int numCells, partitionerStruct_t *partitionerStruct)
{
	unsigned int i;

    partitionerStruct->nodes.clear();

	// For each cell
    for(i = 0; i < numCells; i++)
    {
		// Add a cell
        partitionerStruct->nodes.push_back(nodeStruct_t());
		// Give it an ID
        partitionerStruct->nodes.back().id = i;
        // Null the net pointer
        partitionerStruct->nodes.back().nodeNet = NULL;
    }

    // Add some dummy cells
    for(i = 0; i < (totalCells - numCells); i++)
    {
        // Add a dummy cell
        partitionerStruct->nodes.push_back(nodeStruct_t());
        // Give it an ID
        partitionerStruct->nodes.back().id = numCells + i;
        // Null the net pointer
        partitionerStruct->nodes.back().nodeNet = NULL;
    }
}

void generateNodePlacement(unsigned int numCols, unsigned int numRows, cellPropertiesStruct_t cellProperties, std::vector<std::vector<nodeStruct_t*>> &grid, std::vector<nodeStruct_t> &nodes)
{
	unsigned int i, col, row;

	for (i = 0; i < nodes.size(); i++)
	{
		// Put it somewhere randomly on the grid (make sure it's empty)
		do
		{
			col = static_cast<unsigned int>(getRandomInt(numCols));
			row = static_cast<unsigned int>(getRandomInt(numRows));
		}
        while(grid[col][row] != NULL);

		// Update the cell's position
		updateNodePosition(cellProperties, nodes[i], grid, col, row);
	}
}

void initializeNodeNet(std::vector<netStruct_t> &nets)
{
    unsigned int i, j;

    // Go through the nets and its connections, and update all of the nodes' nets
    for(i = 0; i < nets.size(); i++)
    {
        for(j = 0; j < nets[i].connections.size(); j++)
        {
            nets[i].connections[j]->nodeNet = &nets[i];
        }
    }
}

void initializeNetColors(std::vector<netStruct_t> &nets, unsigned int col, unsigned int row)
{
    unsigned int i, rgb[3];
    sf::Color startingColor;

    // Nets start with a red color
    rgb[0] = 255;
    rgb[1] = 0;
    rgb[2] = 0;

    startingColor = sf::Color(rgb[0], rgb[1], rgb[2], 255);

    // When this function has been called, nets should have been randomly placed on the grid
    // Use their current position to give them their starting
    for(i = 0; i < nets.size(); i++)
    {
        // Calculate the half perimeter of the net, we use the starting position as its "max"
        //nets[i].maxHalfPerim = calculateHalfPerim(nets[i]);
        // Make the maximum half perimeter equal to row + width
        nets[i].maxHalfPerim = col + row - 2;
        // Give the net the starting color
        nets[i].color = startingColor;
    }
}

void updateNetColor(nodeStruct_t &cell)
{
    unsigned int currentHalfPerim, maxHalfPerim, rgb[3];
    sf::Color newColor;
    netStruct_t *netPointer = cell.nodeNet;

    // Nothing to do if the cell doesn't have a net
    if(netPointer == NULL)
    {
        return;
    }

    maxHalfPerim = netPointer->maxHalfPerim;
    // Calculate the half perimeter of the net
    currentHalfPerim = calculateHalfPerim(*netPointer);

    // Interpolate the net's new color
    // Blue| maxHalfPerim -> 1 |Red
    rgb[0] = static_cast<unsigned int>(255.0 * static_cast<double>(currentHalfPerim) / static_cast<double>(maxHalfPerim));
    rgb[1] = 0;
    rgb[2] = static_cast<unsigned int>(255.0 * static_cast<double>(maxHalfPerim - currentHalfPerim) / static_cast<double>(maxHalfPerim));
    newColor = sf::Color(rgb[0], rgb[1], rgb[2], 255);

    //std::cout << maxHalfPerim << " " << currentHalfPerim << std::endl;
    //std::cout << rgb[0] << " ";
    //std::cout << rgb[1] << " ";
    //std::cout << rgb[2] << std::endl;

    // Update the net's color
    netPointer->color = newColor;
}

std::vector<sf::Vertex> generateNetGeometries(std::vector<netStruct_t> &nets)
{
    unsigned int i, j;
    std::vector<sf::Vertex> netLines;
	sf::Color color;
    nodeStruct_t *cellPointer[2];

    for(i = 0; i < nets.size(); i++)
    {
		color = nets[i].color;
        for(j = 0; j < nets[i].connections.size() - 1; j++)
        {
            cellPointer[0] = nets[i].connections[j];
            cellPointer[1] = nets[i].connections[j + 1];
            netLines.push_back(sf::Vertex(sf::Vector2f(cellPointer[0]->drawPos.x, cellPointer[0]->drawPos.y), color));
			netLines.push_back(sf::Vertex(sf::Vector2f(cellPointer[1]->drawPos.x, cellPointer[1]->drawPos.y), color));
        }
    }

    return netLines;
}

std::vector<sf::RectangleShape> generateGridGeometries(cellPropertiesStruct_t cellProperties)
{
    std::vector<sf::RectangleShape> grid;
    unsigned int i, j;
    float rowToColRatio, graphicportRatio, cellSize, cellOffset, cellOppositeOffset;

    // Determine the current row to column ratio
    rowToColRatio = static_cast<float>(cellProperties.numRows)/static_cast<float>(cellProperties.numCols);
    graphicportRatio = WIN_GRAPHICPORT_HEIGHT / WIN_GRAPHICPORT_WIDTH;
    
    // Determine which dimension gets maximized
    if(rowToColRatio > graphicportRatio)
    {
        cellProperties.maximizedDim = DIM_VERTICAL;
    }
    else
    {
        cellProperties.maximizedDim = DIM_HORIZONTAL;
    }

    // Check which orientation gets maximized
    if(cellProperties.maximizedDim == DIM_VERTICAL)
    {
        // Use rows to fill vertically
        cellSize = WIN_GRAPHICPORT_HEIGHT / static_cast<float>(cellProperties.numRows);
        // Cell offset is always half of cell size
        cellOffset = cellSize / 2.f;
        cellOppositeOffset = cellOffset + (WIN_GRAPHICPORT_WIDTH - static_cast<float>(cellProperties.numCols) * cellSize) / 2.f;
    }
    else
    {
        // Use columns to fill horizontally
        cellSize = WIN_GRAPHICPORT_WIDTH / static_cast<float>(cellProperties.numCols);
        // Cell offset is always half of cell size
        cellOffset = cellSize / 2.f;
        cellOppositeOffset = cellOffset + (WIN_GRAPHICPORT_HEIGHT - static_cast<float>(cellProperties.numRows) * cellSize) / 2.f;
    }

    // Save these for use later
    cellProperties.cellSize = cellSize;
    cellProperties.cellOffset = cellOffset;
    cellProperties.cellOppositeOffset = cellOppositeOffset;

    // Populate the grid vector with the data obtained above
    for(i = 0; i < cellProperties.numCols; i++)
    {
        for(j = 0; j < cellProperties.numRows; j++)
        {
            grid.push_back(sf::RectangleShape());
            if(cellProperties.maximizedDim == DIM_VERTICAL)
            {
                grid.back().setPosition(
                    static_cast<float>((i*cellSize) + cellOppositeOffset),
                    static_cast<float>((j*cellSize) + cellOffset)
                );
            }
            else
            {
                grid.back().setPosition(
                    static_cast<float>((i*cellSize) + cellOffset),
                    static_cast<float>((j*cellSize) + cellOppositeOffset)
                );
            }
            grid.back().setSize(sf::Vector2f(cellSize * GRID_SHRINK_FACTOR, cellSize * GRID_SHRINK_FACTOR));
            grid.back().setOrigin(sf::Vector2f(cellSize * GRID_SHRINK_FACTOR * 0.5f, cellSize * GRID_SHRINK_FACTOR * 0.5f));
            grid.back().setFillColor(sf::Color::White);
        }
    }

    return grid;
}

std::vector<sf::RectangleShape> generatePlacedNodeGeometries(std::vector<nodeStruct_t> &cells, dimension_e maximizedDim, float cellSize, float cellOffset, float cellOppositeOffset, state_e state)
{
	std::vector<sf::RectangleShape> placedCells;
	unsigned int i;

	for (i = 0; i < cells.size(); i++)
	{
		// Only look at cells with nets
		if (cells[i].nodeNet == NULL)
		{
			continue;
		}

		placedCells.push_back(sf::RectangleShape());

		if (maximizedDim == DIM_VERTICAL)
		{
			placedCells.back().setPosition(
				static_cast<float>((cells[i].pos.col * cellSize) + cellOppositeOffset),
				static_cast<float>((cells[i].pos.row * cellSize) + cellOffset)
			);
		}
		else
		{
			placedCells.back().setPosition(
				static_cast<float>((cells[i].pos.col * cellSize) + cellOffset),
				static_cast<float>((cells[i].pos.row * cellSize) + cellOppositeOffset)
			);
		}
		placedCells.back().setSize(sf::Vector2f(cellSize * CELL_SHRINK_FACTOR, cellSize * CELL_SHRINK_FACTOR));
		placedCells.back().setOrigin(sf::Vector2f(cellSize * CELL_SHRINK_FACTOR * 0.5f, cellSize * CELL_SHRINK_FACTOR * 0.5f));
		if (state == STATE_PARTITIONING)
		{
			placedCells.back().setFillColor(sf::Color(180, 180, 180, 255));
		}
		else
		{
			placedCells.back().setFillColor(sf::Color(160, 193, 165, 255));
		}
	}

	return placedCells;
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
