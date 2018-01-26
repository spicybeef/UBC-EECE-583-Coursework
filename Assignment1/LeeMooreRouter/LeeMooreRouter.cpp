#define _CRT_SECURE_NO_WARNINGS //  Disable unsafe warnings, to enable use of sprintf within VS2017

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "graphics.h"
#include "LeeMooreRouter.h"

gridStruct_t* grid = new gridStruct_t();

color_types netColors[MAX_NET_COLORS] =
{
    RED, ORANGE, YELLOW, GREEN, DARKGREEN, BLUE, CYAN, MAGENTA
};

int main(int argc, char **argv)
{
    std::string line;
    char * filename = argv[1];
    //const char * filename = "..\\benchmarks\\stdcell.infile";

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
    ParseInputFile(&myfile, grid);
    // Populate cell information
    PopulateCellInfo(grid);
    // Initialize Lee Moore algorithm
    LeeMooreInit(grid);

    /* initialize display with BLACK background */
    init_graphics("Initializing graphics...", BLACK);

    /* still picture drawing allows user to zoom, etc. */
    // Set-up coordinates from (xl,ytop) = (0,0) to 
    // (xr,ybot) = (1000,1000)
    init_world(0., 0., 2000., 2000.);

    // Draw screen a first time
    DrawScreen();

    update_message("Setup complete, ready to start route!");

    // Enable key presses
    set_keypress_input(true);

    event_loop(ActOnButtonPress, ActOnMouseMove, ActOnKeyPress, DrawScreen);

    close_graphics();
    printf("Graphics closed down.\n");

    return (0);
}

bool ParseInputFile(std::ifstream *inputFile, gridStruct_t *outputStruct)
{
    int i, j, numObstructedCells, numNets, numNodesPerNet;
    posStruct_t tempPos;
    std::string line;
    std::vector<std::string> stringVec;

    // 1. Get grid size
    std::getline(*inputFile, line);
    stringVec = SplitString(line, ' ');
    outputStruct->gridSizeX = stoi(stringVec[0]);
    outputStruct->gridSizeY = stoi(stringVec[1]);
    printf("Grid size is %d x %d\n", outputStruct->gridSizeX, outputStruct->gridSizeY);

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
        outputStruct->obstructions.push_back(tempPos);
        printf("\t%d: %d, %d\n", i, outputStruct->obstructions[i].posX, outputStruct->obstructions[i].posY);
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
        outputStruct->nodes.push_back(std::vector<posStruct_t>());
        // 5.2. Iterate through net's nodes and add them
        for(j = 0; j < numNodesPerNet; j++)
        {
            tempPos.posX = stoi(stringVec[1 + 2 * j]);
            tempPos.posY = stoi(stringVec[1 + 2 * j + 1]);
            outputStruct->nodes[i].push_back(tempPos);
            printf("\t\t%d: %d, %d\n", j, outputStruct->nodes[i][j].posX, outputStruct->nodes[i][j].posY);
        }
    }

    return true;
}

bool PopulateCellInfo(gridStruct_t *gridStruct)
{
    unsigned int i, j, currentX, currentY;
    cellStruct_t tempCell;
    std::vector<cellStruct_t> *tempCol;
    cellStruct_t *currentCell;

    //1. Initialize for current grid size
    tempCell.currentCellProp = CELL_EMPTY;
    tempCell.currentNet = -1;
    tempCell.currentNumber = -1;
    tempCell.norNeigh = NULL;
    tempCell.easNeigh = NULL;
    tempCell.souNeigh = NULL;
    tempCell.wesNeigh = NULL;
    for(i = 0; i < gridStruct->gridSizeX; i++)
    {
        tempCol = new std::vector<cellStruct_t>;
        for(j = 0; j < gridStruct->gridSizeY; j++)
        {
            tempCol->push_back(tempCell);
        }
        gridStruct->cells.push_back(*tempCol);
    }

    //2. Populate coordinates and neighbour links
    for(i = 0; i < gridStruct->gridSizeX; i++)
    {
        for(j = 0; j < gridStruct->gridSizeY; j++)
        {
            currentCell = &gridStruct->cells[i][j];

            currentCell->coord.posX = i;
            currentCell->coord.posY = j;

            // Link northern neighbours
            if(j != 0)
            {
                currentCell->norNeigh = &gridStruct->cells[i][j - 1];
            }
            // Link eastern neighbours
            if(i != gridStruct->gridSizeX - 1)
            {
                currentCell->easNeigh = &gridStruct->cells[i + 1][j];
            }
            // Link southern neighbours
            if(j != gridStruct->gridSizeY - 1)
            {
                currentCell->souNeigh = &gridStruct->cells[i][j + 1];
            }
            // Link western neighours
            if(i != 0)
            {
                currentCell->wesNeigh = &gridStruct->cells[i - 1][j];
            }
        }
    }

    //3. Populate obstructions
    for(i = 0; i < gridStruct->obstructions.size(); i++)
    {
        currentX = gridStruct->obstructions[i].posX;
        currentY = gridStruct->obstructions[i].posY;

        currentCell = &gridStruct->cells[currentX][currentY];

        currentCell->currentCellProp = CELL_OBSTRUCTED;
    }

    //4. Populate net sources and sinks
    for(i = 0; i < gridStruct->nodes.size(); i++)
    {
        for(j = 0; j < gridStruct->nodes[i].size(); j++)
        {
            currentX = gridStruct->nodes[i][j].posX;
            currentY = gridStruct->nodes[i][j].posY;

            currentCell = &gridStruct->cells[currentX][currentY];

            currentCell->currentNet = i;

            // If we're the first entry we are a source, otherwise we are a sink
            if(j == 0)
            {
                currentCell->currentCellProp = CELL_NET_SOURCE;
            }
            else
            {
                currentCell->currentCellProp = CELL_NET_SINK;
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
    currentXOrigin = (float)(GRID_PADDING_X + cell->coord.posX * GRID_SIZE_X);
    currentYOrigin = (float)(GRID_PADDING_Y + cell->coord.posY * GRID_SIZE_Y);

    set_draw_mode(DRAW_NORMAL);

    // Depending on the cell type, draw the cell
    switch(cell->currentCellProp)
    {
        case CELL_OBSTRUCTED:
            setcolor(OBSTRUCTION_COLOR);
            fillrect(
                currentXOrigin, currentYOrigin,
                currentXOrigin + GRID_SIZE_X, currentYOrigin + GRID_SIZE_Y
            );
            break;
        case CELL_NET_WIRE:
            setcolor(netColors[cell->currentNet & 7]);
            fillrect(
                currentXOrigin, currentYOrigin,
                currentXOrigin + GRID_SIZE_X, currentYOrigin + GRID_SIZE_Y
            );
            break;
        case CELL_NET_SOURCE:
            setcolor(netColors[cell->currentNet & 7]);
            fillrect(
                currentXOrigin, currentYOrigin,
                currentXOrigin + GRID_SIZE_X, currentYOrigin + GRID_SIZE_Y
            );
            setcolor(WHITE);
            setfontsize(10);
            drawtext(currentXOrigin + 0.5f*GRID_SIZE_X, currentYOrigin + 0.5f*GRID_SIZE_Y, "SRC", 800.);
            break;
        case CELL_NET_SINK:
            setcolor(netColors[cell->currentNet & 7]);
            fillrect(
                currentXOrigin, currentYOrigin,
                currentXOrigin + GRID_SIZE_X, currentYOrigin + GRID_SIZE_Y
            );
            setcolor(WHITE);
            setfontsize(10);
            drawtext(currentXOrigin + 0.5f*GRID_SIZE_X, currentYOrigin + 0.5f*GRID_SIZE_Y, "SNK", 800.);
            break;
        case CELL_EMPTY:
            setcolor(GRID_COLOR);
            drawrect(
                currentXOrigin, currentYOrigin,
                currentXOrigin + GRID_SIZE_X, currentYOrigin + GRID_SIZE_Y
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
        drawtext(currentXOrigin + 0.5f*GRID_SIZE_X, currentYOrigin + 0.5f*GRID_SIZE_Y, strBuff, 800.);
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

    for(i = 0; i < grid->gridSizeX; i++)
    {
        for(j = 0; j < grid->gridSizeY; j++)
        {
            // Draw cell
            DrawCell(&grid->cells[i][j]);
        }
    }
}

void LeeMooreInit(gridStruct_t *gridStruct)
{
    // Initialize algorithm state and starting net
    gridStruct->currentRoutingState = STATE_LM_IDLE;
    gridStruct->currentNet = 0;
}

void LeeMooreExec(gridStruct_t *gridStruct, stepType_e stepType)
{

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
            // Populate cell information
            PopulateCellInfo(grid);
            // Initialize Lee Moore algorithm
            LeeMooreInit(grid);
            break;
        case 'N':
            printf("Taking a single step...\n");
            LeeMooreExec(grid, STEP_SINGLE);
            break;
        case 'M':
            printf("Attempting to route a single net...\n");
            LeeMooreExec(grid, STEP_NET);
            break;
        case 'A':
            printf("Attempting to route the entire grid...\n");
            LeeMooreExec(grid, STEP_COMPLETE);
        default:
            break;
    }

    // redraw screen
    DrawScreen();
}
