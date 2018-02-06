#define _CRT_SECURE_NO_WARNINGS //  Disable unsafe warnings, to enable use of sprintf within VS2017

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <ctime>
#include <cstdlib>

#include "graphics.h"
#include "LeeMooreRouter.h"

parsedInputStruct_t *input = new parsedInputStruct_t();
gridStruct_t *grid = new gridStruct_t();

// Globals used to center grid in window
// TODO: find a better way to do this
int cellSizeX, cellSizeY, gridMarginX, gridMarginY;

color_types netColors[MAX_NET_COLORS] =
{
    RED, ORANGE, YELLOW, GREEN, DARKGREEN, BLUE, CYAN, MAGENTA
};

int main(int argc, char **argv)
{
    char * filename = argv[1];
    //const char * filename = "..\\benchmarks\\kuma.infile";

    // Filename to read in is the second argument
    std::ifstream myfile(filename, std::ios::in);

    // Check if file was opened properly
    if(myfile.is_open())
    {
        printf("File %s opened! Here's what's in it:\n", filename);
    }
    else
    {
        printf("FATAL ERROR, file %s couldn't be opened!\n", filename);
        return -1;
    }

    // Parse input file
    ParseInputFile(&myfile, input);
    // Initialize Lee Moore algorithm
    LeeMooreInit(input, grid);

    // Scale cells and padding to current grid
    cellSizeX = 1280 / (input->gridSizeX + 4);
    cellSizeY = cellSizeX;
    gridMarginX = cellSizeX * 2;
    gridMarginY = gridMarginX;

    /* initialize display with BLACK background */
    init_graphics("Router", BLACK);

    /* still picture drawing allows user to zoom, etc. */
    // Set-up coordinates from (xl,ytop) = (0,0) to 
    // (xr,ybot) = (1000,1000)
    // init_graphics scaled the window based on a factor of the current resolution
    // TODO: make it dynamic
    init_world(0., 0., 1280., 864.);

    // Draw screen a first time
    DrawScreen();

    update_message("Setup complete, ready to start route!");

    // Enable key presses
    set_keypress_input(true);

    // Start main event loop
    event_loop(ActOnButtonPress, ActOnMouseMove, ActOnKeyPress, DrawScreen);

    close_graphics();
    printf("Graphics closed down.\n");

    return (0);
}

// random generator function:
int MyRandomInt(int i)
{
    return std::rand() % i;
}

bool ParseInputFile(std::ifstream *inputFile, parsedInputStruct_t *inputStruct)
{
    int i, j, numObstructedCells, numNets, numNodesPerNet;
    posStruct_t tempPos;
    std::string line;
    std::vector<std::string> stringVec;

    // 1. Get grid size
    std::getline(*inputFile, line);
    stringVec = SplitString(line, ' ');
    inputStruct->gridSizeX = stoi(stringVec[0]);
    inputStruct->gridSizeY = stoi(stringVec[1]);
    printf("Grid size is %d x %d\n", inputStruct->gridSizeX, inputStruct->gridSizeY);

    // 2. Determine the amount of obstructed cells
    std::getline(*inputFile, line);
    numObstructedCells = stoi(line);
    printf("%d obstructed cells in total:\n", numObstructedCells);

    // 3. Get obstructed cell locations
    for(i = 0; i < numObstructedCells; i++)
    {
        std::getline(*inputFile, line);
        stringVec = SplitString(line, ' ');
        tempPos.posX = stoi(stringVec[0]);
        tempPos.posY = stoi(stringVec[1]);
        inputStruct->obstructions.push_back(tempPos);
        printf("\t%d: %d, %d\n", i, inputStruct->obstructions[i].posX, inputStruct->obstructions[i].posY);
    }

    // 4. Get number of nets to route
    std::getline(*inputFile, line);
    numNets = stoi(line);
    printf("%d nets in total:\n", numNets);

    // 5. Get nets to route
    for(i = 0; i < numNets; i++)
    {
        std::getline(*inputFile, line);
        stringVec = SplitString(line, ' ');
        // 5.1. Get number of nodes for this net
        numNodesPerNet = stoi(stringVec[0]);
        printf("\t%d: %d nodes:\n", i, numNodesPerNet);
        inputStruct->nodes.push_back(std::vector<posStruct_t>());
        // 5.2. Iterate through net's nodes and add them
        for(j = 0; j < numNodesPerNet; j++)
        {
            tempPos.posX = stoi(stringVec[1 + 2 * j]);
            tempPos.posY = stoi(stringVec[1 + 2 * j + 1]);
            inputStruct->nodes[i].push_back(tempPos);
            printf("\t\t%d: %d, %d\n", j, inputStruct->nodes[i][j].posX, inputStruct->nodes[i][j].posY);
        }
    }

    return true;
}

bool PopulateCellInfo(parsedInputStruct_t *parsedInputStruct, gridStruct_t *gridStruct)
{
    unsigned int i, j, currentX, currentY;
    cellStruct_t tempCell;
    std::vector<cellStruct_t> *tempCol;
    cellStruct_t *currentCell;
    std::srand(unsigned(std::time(0)));

    //1. Initialize for current grid size
    tempCell.currentCellProp = CELL_EMPTY;
    tempCell.currentNet = -1;
    tempCell.currentNumber = -1;
    tempCell.neighbours[DIR_NORTH] = NULL;
    tempCell.neighbours[DIR_EAST] = NULL;
    tempCell.neighbours[DIR_SOUTH] = NULL;
    tempCell.neighbours[DIR_WEST] = NULL;
    gridStruct->cells.clear();
    for(i = 0; i < parsedInputStruct->gridSizeX; i++)
    {
        tempCol = new std::vector<cellStruct_t>;
        for(j = 0; j < parsedInputStruct->gridSizeY; j++)
        {
            tempCol->push_back(tempCell);
        }
        gridStruct->cells.push_back(*tempCol);
    }

    //2. Populate coordinates and neighbour links
    for(i = 0; i < parsedInputStruct->gridSizeX; i++)
    {
        for(j = 0; j < parsedInputStruct->gridSizeY; j++)
        {
            currentCell = &gridStruct->cells[i][j];

            currentCell->coord.posX = i;
            currentCell->coord.posY = j;

            // Link northern neighbours
            if(j != 0)
            {
                currentCell->neighbours[DIR_NORTH] = &gridStruct->cells[i][j - 1];
            }
            // Link eastern neighbours
            if(i != parsedInputStruct->gridSizeX - 1)
            {
                currentCell->neighbours[DIR_EAST] = &gridStruct->cells[i + 1][j];
            }
            // Link southern neighbours
            if(j != parsedInputStruct->gridSizeY - 1)
            {
                currentCell->neighbours[DIR_SOUTH] = &gridStruct->cells[i][j + 1];
            }
            // Link western neighours
            if(i != 0)
            {
                currentCell->neighbours[DIR_WEST] = &gridStruct->cells[i - 1][j];
            }
        }
    }

    //3. Populate obstructions
    for(i = 0; i < parsedInputStruct->obstructions.size(); i++)
    {
        currentX = parsedInputStruct->obstructions[i].posX;
        currentY = parsedInputStruct->obstructions[i].posY;

        currentCell = &gridStruct->cells[currentX][currentY];

        currentCell->currentCellProp = CELL_OBSTRUCTED;
    }

    //4. Populate net sources and sinks
    for(i = 0; i < parsedInputStruct->nodes.size(); i++)
    {
        for(j = 0; j < parsedInputStruct->nodes[i].size(); j++)
        {
            currentX = parsedInputStruct->nodes[i][j].posX;
            currentY = parsedInputStruct->nodes[i][j].posY;

            currentCell = &gridStruct->cells[currentX][currentY];

            currentCell->currentNet = i;

            // If we're the first entry we are a source, otherwise we are a sink
            if(j == 0)
            {
                currentCell->currentCellProp = CELL_NET_SOURCE;
            }
            else
            {
                currentCell->currentCellProp = CELL_NET_SINK_UNCONN;
            }
        }
    }

    return true;
}

std::vector<std::string> SplitString(std::string inString, char delimiter)
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

void DrawCell(cellStruct_t *cell)
{
    float currentXOrigin, currentYOrigin;
    char strBuff[80];

    // Make things clean by setting our origin here
    currentXOrigin = (float)(gridMarginX + cell->coord.posX * cellSizeX);
    currentYOrigin = (float)(gridMarginY + cell->coord.posY * cellSizeY);

    set_draw_mode(DRAW_NORMAL);

    // Depending on the cell type, draw the cell
    switch(cell->currentCellProp)
    {
        case CELL_OBSTRUCTED:
            setcolor(OBSTRUCTION_COLOR);
            fillrect(
                currentXOrigin, currentYOrigin,
                currentXOrigin + cellSizeX, currentYOrigin + cellSizeY
            );
            break;
        case CELL_NET_WIRE_UNCONN:
            setcolor(netColors[cell->currentNet & 7]);
            fillrect(
                currentXOrigin, currentYOrigin,
                currentXOrigin + cellSizeX, currentYOrigin + cellSizeY
            );
            setcolor(WHITE);
            setfontsize(10);
            drawtext(currentXOrigin + 0.5f*cellSizeX, currentYOrigin + 0.5f*cellSizeY, "X", 800.);
            break;
        case CELL_NET_WIRE_CONN:
            setcolor(netColors[cell->currentNet & 7]);
            fillrect(
                currentXOrigin, currentYOrigin,
                currentXOrigin + cellSizeX, currentYOrigin + cellSizeY
            );
            break;
        case CELL_NET_SOURCE:
            setcolor(netColors[cell->currentNet & 7]);
            fillrect(
                currentXOrigin, currentYOrigin,
                currentXOrigin + cellSizeX, currentYOrigin + cellSizeY
            );
            setcolor(WHITE);
            setfontsize(10);
            drawtext(currentXOrigin + 0.5f*cellSizeX, currentYOrigin + 0.5f*cellSizeY, "SRC", 800.);
            break;
        case CELL_NET_SINK_UNCONN:
            setcolor(netColors[cell->currentNet & 7]);
            fillrect(
                currentXOrigin, currentYOrigin,
                currentXOrigin + cellSizeX, currentYOrigin + cellSizeY
            );
            setcolor(WHITE);
            setfontsize(10);
            drawtext(currentXOrigin + 0.5f*cellSizeX, currentYOrigin + 0.5f*cellSizeY, "SNK_X", 800.);
            break;
        case CELL_NET_SINK_CONN:
            setcolor(netColors[cell->currentNet & 7]);
            fillrect(
                currentXOrigin, currentYOrigin,
                currentXOrigin + cellSizeX, currentYOrigin + cellSizeY
            );
            setcolor(WHITE);
            setfontsize(10);
            drawtext(currentXOrigin + 0.5f*cellSizeX, currentYOrigin + 0.5f*cellSizeY, "SNK", 800.);
            break;
        case CELL_EMPTY:
            setcolor(GRID_COLOR);
            drawrect(
                currentXOrigin, currentYOrigin,
                currentXOrigin + cellSizeX, currentYOrigin + cellSizeY
            );
            break;
        default:
            // Should never be in here
            break;
    }

    // During expansion, show the number
    if(cell->currentNumber > 0)
    {
        sprintf(strBuff, "%d", cell->currentNumber);
        setcolor(WHITE);
        setfontsize(10);
        drawtext(currentXOrigin + 0.5f*cellSizeX, currentYOrigin + 0.5f*cellSizeY, strBuff, 800.);
    }
}

void DrawScreen(void)
{
    unsigned int i, j;

    set_draw_mode(DRAW_NORMAL);
    clearscreen();  /* Should precede drawing for all drawscreens */

    // Draw grid on screen
    setlinestyle(SOLID);
    setlinewidth(0);

    for(i = 0; i < input->gridSizeX; i++)
    {
        for(j = 0; j < input->gridSizeY; j++)
        {
            // Draw cell
            DrawCell(&grid->cells[i][j]);
        }
    }
}

void LeeMooreInit(parsedInputStruct_t *parsedInputStruct, gridStruct_t *gridStruct)
{
    unsigned int i;
    // Shuffle the net order
    std::random_shuffle(input->nodes.begin(), input->nodes.end(), MyRandomInt);
    // Populate cell information
    PopulateCellInfo(input, grid);
    // Initialize algorithm state and starting net
    gridStruct->currentRoutingState = STATE_LM_IDLE;
    gridStruct->currentNet = 0;
    gridStruct->currentExpansion = 0;
    // Clear out the expansion list
    gridStruct->expansionList.clear();
    // Clear our our last route
    gridStruct->lastRoute.clear();
    // Clear the last cell pointer
    gridStruct->lastCell = NULL;
    // Reset our net route counts
    // For each net, we will only have to route nodes - 1 since we don't count the source
    gridStruct->netRoutedNodes.clear();
    for(i = 0; i < parsedInputStruct->nodes.size(); i++)
    {
        gridStruct->netRoutedNodes.push_back(parsedInputStruct->nodes[i].size() - 1);
    }
}

void LeeMooreExec(parsedInputStruct_t *parsedInputStruct, gridStruct_t *gridStruct, stepType_e stepType)
{
    char strBuff[80];
    bool doneExpansion;
    bool doneWalkback;
    bool keepRouting;
    unsigned int x, y, i, dir, currentNet;
    cellStruct_t* currentCell;
    std::vector<cellStruct_t*> *currentExpansionList;

    // Initially we don't keep routing
    keepRouting = false;

    // Keep track of our current net
    currentNet = gridStruct->currentNet;

    do
    {
        // Execute routing step based on current state
        switch(gridStruct->currentRoutingState)
        {
            case STATE_LM_IDLE:
                // Ready to route! Go to expansion...
                sprintf(strBuff, "Ready to route! Next net: %d", gridStruct->currentNet);
                update_message(strBuff);
                gridStruct->currentRoutingState = STATE_LM_EXPANSION;
                break;
            case STATE_LM_EXPANSION:
                doneExpansion = false;
                // Expansion state for current net
                sprintf(strBuff, "Currently expanding net: %d layer: %d", gridStruct->currentNet, gridStruct->currentExpansion);
                update_message(strBuff);
                // Push a new expansion list for this layer
                currentExpansionList = new std::vector<cellStruct_t*>;
                gridStruct->expansionList.push_back(*currentExpansionList);

                // If we are at our first expansion and not our first node, give the last route cells an expansion of 0 and add them to the expansion list
                // (If we're on the first node, then we still have nodes - 1 to route, anything less and we're on subsequent routes)
                if(gridStruct->currentExpansion == 0 &&
                    gridStruct->netRoutedNodes[gridStruct->currentNet] < (parsedInputStruct->nodes[gridStruct->currentNet].size() - 1))
                {
                    // Add the last route to the expansion list and make them all an expansion of 0
                    for(i = 0; i < gridStruct->lastRoute.size(); i++)
                    {
                        gridStruct->lastRoute[i]->currentNumber = 0;
                        gridStruct->expansionList[gridStruct->currentExpansion].push_back(gridStruct->lastRoute[i]);
                    }
                }
                // If we are at our first expansion and our first node, give the source the expansion of 0 and add it to the list
                else if(gridStruct->currentExpansion == 0)
                {
                    // First node is the source
                    x = parsedInputStruct->nodes[gridStruct->currentNet][0].posX;
                    y = parsedInputStruct->nodes[gridStruct->currentNet][0].posY;
                    gridStruct->expansionList[gridStruct->currentExpansion].push_back(&gridStruct->cells[x][y]);
                    // Give our source an expansion of 0
                    gridStruct->cells[x][y].currentNumber = 0;
                }
                // We've started expanding already
                else
                {
                    printf("Cells to visit for expansion: %d\n", gridStruct->expansionList[gridStruct->currentExpansion - 1].size());
                    // Check if we can still expand, if not, we failed this route
                    if(gridStruct->expansionList[gridStruct->currentExpansion - 1].size() == 0)
                    {
                        // Reset our expansion
                        ResetCellExpansion(gridStruct);
                        gridStruct->currentRoutingState = STATE_LM_ROUTE_FAILURE;
                        doneExpansion = true;
                        break;
                    }
                    // For each cell in the previous layer's expansion list, expand into the new (current) expansion
                    for(i = 0; i < gridStruct->expansionList[gridStruct->currentExpansion - 1].size(); i++)
                    {
                        // Get a pointer to the current cell
                        currentCell = gridStruct->expansionList[gridStruct->currentExpansion - 1][i];

                        // For each cardinal direction
                        for(dir = DIR_NORTH; dir < DIR_NUM; dir++)
                        {
                            // Make sure we have a cell to look at
                            if(currentCell->neighbours[dir] == NULL)
                            {
                                continue;
                            }
                            // Check if the cell is an unconnected sink and it's our net
                            else if(currentCell->neighbours[dir]->currentCellProp == CELL_NET_SINK_UNCONN &&
                                currentCell->neighbours[dir]->currentNet == gridStruct->currentNet)
                            {
                                // We've found a sink! Keep a reference to it
                                gridStruct->lastCell = currentCell->neighbours[dir];
                                // Time to walk back from it now
                                gridStruct->currentRoutingState = STATE_LM_WALKBACK;
                                // No more expansion, lets get out of here
                                doneExpansion = true;
                                break;
                            }
                            // Check if the cell is routeable (it's empty and isn't part of a routing layer
                            else if(currentCell->neighbours[dir]->currentCellProp == CELL_EMPTY && currentCell->neighbours[dir]->currentNumber == -1)
                            {
                                // We found a routeable cell! Mark it for the current expansion
                                currentCell->neighbours[dir]->currentNumber = gridStruct->currentExpansion;
                                // Add a reference to it for the current expansion list
                                gridStruct->expansionList[gridStruct->currentExpansion].push_back(currentCell->neighbours[dir]);
                            }
                        }

                        // Break out of outer loop since we've found our sink and we're done expansion
                        if(doneExpansion)
                        {
                            break;
                        }
                    }
                }
                // Go to the next expansion layer if we're not done expanding
                if(!doneExpansion)
                {
                    gridStruct->currentExpansion++;
                }
                break;
            case STATE_LM_WALKBACK:
                doneWalkback = false;
                // Walkback state for current net
                sprintf(strBuff, "Currently walking back net: %d layer: %d", gridStruct->currentNet, gridStruct->currentExpansion);
                update_message(strBuff);

                // At this point we have found a sink to connect to, now we need to walk back
                // The lastcell pointer will have the last cell to walk back from

                // Get a pointer to the last cell
                currentCell = gridStruct->lastCell;
                printf("Walking back net %d, current cell is %d, %d\n", gridStruct->currentNet, currentCell->coord.posX, currentCell->coord.posY);

                // Add the cell to our last route list
                gridStruct->lastRoute.push_back(currentCell);

                // For each cardinal direction
                for(dir = DIR_NORTH; dir < DIR_NUM; dir++)
                {
                    // Make sure we have a cell to look at
                    if(currentCell->neighbours[dir] == NULL)
                    {
                        continue;
                    }
                    // Check if we've found our original net
                    else if(currentCell->neighbours[dir]->currentNet == gridStruct->currentNet && !(currentCell->neighbours[dir]->currentCellProp == CELL_NET_SINK_UNCONN || currentCell->neighbours[dir]->currentCellProp == CELL_NET_WIRE_UNCONN))
                    {
                        // Found our net!
                        printf("Found our net!\n");
                        // Also add it to our last route
                        gridStruct->lastRoute.push_back(currentCell->neighbours[dir]);
                        // Time to cleanup
                        // Go into our last route, and change unconnected sinks and wires to connected
                        for(i = 0; i < gridStruct->lastRoute.size(); i++)
                        {
                            switch(gridStruct->lastRoute[i]->currentCellProp)
                            {
                                case CELL_NET_SINK_UNCONN:
                                    gridStruct->lastRoute[i]->currentCellProp = CELL_NET_SINK_CONN;
                                    break;
                                case CELL_NET_WIRE_UNCONN:
                                    gridStruct->lastRoute[i]->currentCellProp = CELL_NET_WIRE_CONN;
                                    break;
                                default:
                                    break;
                            }
                        }
                        // Go into our expansion list and revert their numbers to -1
                        ResetCellExpansion(gridStruct);
                        // Clear out the expansion list
                        gridStruct->expansionList.clear();
                        // Go back to expansion of 0
                        gridStruct->currentExpansion = 0;
                        // Done our walkback
                        doneWalkback = true;
                        // Decrement our nodes to route
                        gridStruct->netRoutedNodes[gridStruct->currentNet]--;
                        // If we have nodes left to route, go back to expansion
                        if(gridStruct->netRoutedNodes[gridStruct->currentNet])
                        {
                            gridStruct->currentRoutingState = STATE_LM_EXPANSION;
                        }
                        // Otherwise we are done with this net
                        else
                        {
                            // Update the screen
                            DrawScreen();
                            // Clear the last route
                            gridStruct->lastRoute.clear();
                            gridStruct->currentNet++;
                            // Save this grid if it's our best yet
                            if(gridStruct->currentNet > gridStruct->bestNetsRouted)
                            {
                                gridStruct->bestNetsRouted = gridStruct->currentNet;
                                gridStruct->bestGrid = gridStruct->cells;
                            }
                            // Check if this was our last net
                            if(gridStruct->currentNet == parsedInputStruct->nodes.size())
                            {
                                // If so, we're done!
                                gridStruct->currentRoutingState = STATE_LM_ROUTE_SUCCESS;
                            }
                            else
                            {
                                // More nets to route...
                                gridStruct->currentRoutingState = STATE_LM_EXPANSION;
                            }
                        }
                        break;
                    }
                    // Check if the cell's number is one less than the current expansion
                    else if(currentCell->neighbours[dir]->currentNumber == gridStruct->currentExpansion - 1)
                    {
                        // We've found a route back!
                        // Take note of our cell
                        gridStruct->lastCell = currentCell->neighbours[dir];
                        // Route the cell
                        currentCell->neighbours[dir]->currentNet = gridStruct->currentNet;
                        currentCell->neighbours[dir]->currentCellProp = CELL_NET_WIRE_UNCONN;
                        // Go back an expansion
                        gridStruct->currentExpansion--;
                        // We're done, go to next walkback cell
                        break;
                    }
                }

                break;
            case STATE_LM_ROUTE_FAILURE:
                // We failed the last route, don't keep routing :(
                sprintf(strBuff, "Route failed on net %d!", gridStruct->currentNet);
                update_message(strBuff);
                gridStruct->cells = gridStruct->bestGrid;
                printf("Route failed - showing best grid!\n");
                keepRouting = false;
                break;
            case STATE_LM_ROUTE_SUCCESS:
                // We've successfully routed! Yay!
                sprintf(strBuff, "Route SUCCESS after trying %d time(s)", gridStruct->currentRetries + 1);
                update_message(strBuff);
                printf("Route SUCCESS!\n");
                keepRouting = false;
                break;
            default:
                break;
        }
        // If we're routing an entire net and we're still on the same name net, keep going if we have failed/succeeded yet
        if(!(gridStruct->currentRoutingState == STATE_LM_ROUTE_FAILURE || gridStruct->currentRoutingState == STATE_LM_ROUTE_SUCCESS))
        {
            switch(stepType)
            {
                case STEP_SINGLE:
                    keepRouting = false;
                    break;
                case STEP_NET:
                    if(currentNet == gridStruct->currentNet)
                    {
                        keepRouting = true;
                    }
                    // Finished this net
                    else
                    {
                        keepRouting = false;
                    }
                    break;
                case STEP_COMPLETE:
                    keepRouting = true;
                    break;
                default:
                    keepRouting = false;
                    break;
            }
        }
        // If we've failed but we have more retries, rip up routed nets and try again
        else if(gridStruct->currentRoutingState == STATE_LM_ROUTE_FAILURE && gridStruct->currentRetries < MAXIMUM_ROUTING_RETRIES)
        {
            keepRouting = true;
            gridStruct->currentRetries++;
            LeeMooreInit(parsedInputStruct, gridStruct);
        }
    }
    while(keepRouting);
}

void ResetCellExpansion(gridStruct_t *gridStruct)
{
    unsigned int i, j;

    for(i = 0; i < gridStruct->expansionList.size(); i++)
    {
        for(j = 0; j < gridStruct->expansionList[i].size(); j++)
        {
            gridStruct->expansionList[i][j]->currentNumber = -1;
        }
    }
}

void ActOnButtonPress(float x, float y)
{
    /* Called whenever event_loop gets a button press in the graphics *
     * area.  Allows the user to do whatever he/she wants with button *
     * clicks.                                                        */
    printf("User clicked a button at coordinates (%f, %f)\n", x, y);
}

void ActOnMouseMove(float x, float y)
{
    // function to handle mouse move event, the current mouse position in the current world coordinate
    // as defined as MAX_X and MAX_Y in init_world is returned
    printf("Mouse move at (%f,%f)\n", x, y);
}

void ActOnKeyPress(char c)
{
    // function to handle keyboard press event, the ASCII character is returned
    printf("Key press: %c\n", c);

    switch(c)
    {
        case 'R':
            printf("Resetting grid! \n");
            // Initialize Lee Moore algorithm
            LeeMooreInit(input, grid);
            break;
        case 'N':
            printf("Taking a single step...\n");
            LeeMooreExec(input, grid, STEP_SINGLE);
            break;
        case 'M':
            printf("Attempting to route a single net...\n");
            LeeMooreExec(input, grid, STEP_NET);
            break;
        case 'A':
            printf("Attempting to route the entire grid...\n");
            LeeMooreExec(input, grid, STEP_COMPLETE);
        default:
            break;
    }

    // redraw screen
    DrawScreen();
}
