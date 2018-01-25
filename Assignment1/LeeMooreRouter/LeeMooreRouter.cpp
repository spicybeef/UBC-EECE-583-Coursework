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
    try
    {
        std::string line;
        //char * filename = argv[1];
        const char * filename = "..\\benchmarks\\stdcell.infile";

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

        // Parse grid struct
        ParseGridStruct(&myfile, grid);

        /* initialize display with BLACK background */
        printf("About to start graphics.\n");
        init_graphics("Some Example Graphics", BLACK);

        /* still picture drawing allows user to zoom, etc. */
        // Set-up coordinates from (xl,ytop) = (0,0) to 
        // (xr,ybot) = (1000,1000)
        init_world(0., 0., 2000., 2000.);
        update_message("Interactive graphics example.");

        DrawScreen();

        event_loop(ActOnButtonPress, ActOnMouseMove, ActOnKeyPress, DrawScreen);

        close_graphics();
        printf("Graphics closed down.\n");

        return (0);
    }
    catch(const std::exception & ex)
    {
        std::cerr << ex.what() << std::endl;
    }
}

bool ParseGridStruct(std::ifstream *inputFile, gridStruct_t *outputStruct)
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
        outputStruct->obstruction.push_back(tempPos);
        printf("\t%d: %d, %d\n", i, outputStruct->obstruction[i].posX, outputStruct->obstruction[i].posY);
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

void DrawScreen(void)
{
    unsigned int i, j;

    set_draw_mode(DRAW_NORMAL);  // Should set this if your program does any XOR drawing in callbacks.
    clearscreen();  /* Should precede drawing for all drawscreens */

    // Draw grid on screen
    setlinestyle(SOLID);
    setlinewidth(0);
    setcolor(GRID_COLOR);
    for(i = 0; i < grid->gridSizeX; i++)
    {
        for(j = 0; j < grid->gridSizeY; j++)
        {
            drawrect(
                (float) (GRID_PADDING_X + i*GRID_SIZE_X), (float) (GRID_PADDING_Y + j*GRID_SIZE_Y),
                (float) (GRID_PADDING_X + (i+1)*GRID_SIZE_X), (float) (GRID_PADDING_Y + (j+1)*GRID_SIZE_Y)
            );
        }
    }

    // Draw obstructions
    setcolor(OBSTRUCTION_COLOR);
    for(i = 0; i < grid->obstruction.size(); i++)
    {
        fillrect(
            (float)(GRID_PADDING_X + grid->obstruction[i].posX * GRID_SIZE_X), (float)(GRID_PADDING_Y + grid->obstruction[i].posY * GRID_SIZE_Y),
            (float)(GRID_PADDING_X + (grid->obstruction[i].posX+1) * GRID_SIZE_X), (float)(GRID_PADDING_Y + (grid->obstruction[i].posY+1) * GRID_SIZE_Y)
        );
    }

    // Draw net sources and sinks
    for(i = 0; i < grid->nodes.size(); i++)
    {
        // Set net's color
        setcolor(netColors[i & 7]);
        //setcolor(RED);
        for(j = 0; j < grid->nodes[i].size(); j++)
        {
            // Draw each net's node's
            fillrect(
                (float)(GRID_PADDING_X + grid->nodes[i][j].posX * GRID_SIZE_X), (float)(GRID_PADDING_Y + grid->nodes[i][j].posY * GRID_SIZE_Y),
                (float)(GRID_PADDING_X + (grid->nodes[i][j].posX + 1) * GRID_SIZE_X), (float)(GRID_PADDING_Y + (grid->nodes[i][j].posY + 1) * GRID_SIZE_Y)
            );
        }
    }
}


void Delay(void)
{
    /* A simple delay routine for animation. */

    int i, j, k, sum;

    sum = 0;
    for(i = 0; i < 100; i++)
        for(j = 0; j < i; j++)
            for(k = 0; k < 1000; k++)
                sum = sum + i + j - k;
}


void ActOnNewButtonFunc(void(*drawscreen_ptr) (void))
{
    //char old_button_name[200], new_button_name[200];
    //printf("You pressed the new button!\n");
    //setcolor(MAGENTA);
    //setfontsize(12);
    //drawtext(500., 500., "You pressed the new button!", 10000.);
    //sprintf(old_button_name, "%d Clicks", num_new_button_clicks);
    //num_new_button_clicks++;
    //sprintf(new_button_name, "%d Clicks", num_new_button_clicks);
    //change_button_text(old_button_name, new_button_name);
}


void ActOnButtonPress(float x, float y)
{
    /* Called whenever event_loop gets a button press in the graphics *
     * area.  Allows the user to do whatever he/she wants with button *
     * clicks.                                                        */

    printf("User clicked a button at coordinates (%f, %f)\n", x, y);

    //if(line_entering_demo)
    //{
    //	if(rubber_band_on)
    //	{
    //		rubber_band_on = false;
    //		x2 = x;
    //		y2 = y;
    //		have_entered_line = true;  // both endpoints clicked on --> consider entered.

    //		// Redraw screen to show the new line.  Could do incrementally, but this is easier.
    //		DrawScreen();
    //	}
    //	else
    //	{
    //		rubber_band_on = true;
    //		x1 = x;
    //		y1 = y;
    //		have_entered_line = false;
    //		have_rubber_line = false;
    //	}
    //}

}



void ActOnMouseMove(float x, float y)
{
    // function to handle mouse move event, the current mouse position in the current world coordinate
    // as defined as MAX_X and MAX_Y in init_world is returned

    printf("Mouse move at (%f,%f)\n", x, y);
    //if(rubber_band_on)
    //{
    //	// Go into XOR mode.  Make sure we set the linestyle etc. for xor mode, since it is 
    //	// stored in different state than normal mode.
    //	set_draw_mode(DRAW_XOR);
    //	setlinestyle(SOLID);
    //	setcolor(WHITE);
    //	setlinewidth(1);

    //	if(have_rubber_line)
    //	{
    //		// Erase old line.  
    //		drawline(x1, y1, x2, y2);
    //	}
    //	have_rubber_line = true;
    //	x2 = x;
    //	y2 = y;
    //	drawline(x1, y1, x2, y2);  // Draw new line
    //}
}


void ActOnKeyPress(char c)
{
    // function to handle keyboard press event, the ASCII character is returned
    printf("Key press: %c\n", c);
}
