#define _CRT_SECURE_NO_WARNINGS //  Disable unsafe warnings, to enable use of sprintf within VS2017

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "graphics.h"
#include "LeeMooreRouter.h"

gridStruct_t grid;

int main(int argc, char **argv)
{
    std::string line;
    char * filename = argv[1];

    // Clear out grid struct
    memset(&grid, 0xFF, sizeof(grid));

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
    ParseGridStruct(&myfile, &grid);

    /* initialize display with BLACK background */
    printf("About to start graphics.\n");
    init_graphics("Some Example Graphics", BLACK);

    /* still picture drawing allows user to zoom, etc. */
    // Set-up coordinates from (xl,ytop) = (0,0) to 
    // (xr,ybot) = (1000,1000)
    init_world(0., 0., 10000., 10000.);
    update_message("Interactive graphics example.");

    DrawScreen();

    // Pass control to the window handling routine.  It will watch for user input
    // and redraw the screen / pan / zoom / etc. the graphics in response to user
    // input or windows being moved around the screen.  This is done by calling the
    // four callbacks below.  mouse movement and key press (keyboard) events aren't 
    // enabled by default, so they aren't on yet.
    event_loop(ActOnButtonPress, ActOnMouseMove, NULL, DrawScreen);

    ///* animation section */
    //clearscreen();
    //update_message("Non-interactive (animation) graphics example.");
    //setcolor(RED);
    //setlinewidth(1);
    //setlinestyle(DASHED);
    //init_world(0., 0., 1000., 1000.);
    //for(i = 0; i < 50; i++)
    //{
    //	drawline((float)i, (float)(10.*i), (float)(i + 500.), (float)(10.*i + 10.));
    //	flushinput();
    //	Delay();
    //}

    ///* Draw an interactive still picture again.  I'm also creating one new button. */

    //init_world(0., 0., 1000., 1000.);
    //update_message("Interactive graphics #2. Click in graphics area to rubber band line.");
    //create_button("Window", "0 Clicks", ActOnNewButtonFunc);

    //// Enable mouse movement (not just button presses) and key board input.
    //// The appropriate callbacks will be called by event_loop.
    //set_keypress_input(true);
    //set_mouse_move_input(true);
    ////line_entering_demo = true;

    //// draw the screen once before calling event loop, so the picture is correct 
    //// before we get user input.
    //DrawScreen();
    //event_loop(ActOnButtonPress, ActOnMouseMove, ActOnKeyPress, DrawScreen);

    close_graphics();
    printf("Graphics closed down.\n");

    return (0);
}

bool ParseGridStruct(std::ifstream *inputFile, gridStruct_t *outputStruct)
{
    int i, j;
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
    outputStruct->numObstructedCells = stoi(line);
    printf("%d obstructed cells in total:\n", outputStruct->numObstructedCells);

    // 3. Get obstructed cell locations
    for(i = 0; i < outputStruct->numObstructedCells; i++)
    {
        std::getline(*inputFile, line);
        stringVec = SplitString(line, ' ');
        outputStruct->obstructedX[i] = stoi(stringVec[0]);
        outputStruct->obstructedY[i] = stoi(stringVec[1]);
        printf("\t%d: %d, %d\n", i, outputStruct->obstructedX[i], outputStruct->obstructedY[i]);
    }

    // 4. Get number of nets to route
    std::getline(*inputFile, line);
    outputStruct->numNets = stoi(line);
    printf("%d nets in total:\n", outputStruct->numNets);

    // 5. Get nets to route
    for(i = 0; i < outputStruct->numNets; i++)
    {
        std::getline(*inputFile, line);
        stringVec = SplitString(line, ' ');
        // 5.1. Get number of nodes for this net
        outputStruct->nodesPerNet[i] = stoi(stringVec[0]);
        printf("\t%d: %d nodes:\n", i, outputStruct->nodesPerNet[i]);
        // 5.2. Iterate through net's nodes and add them
        for(j = 0; j < outputStruct->nodesPerNet[i]; j++)
        {
            outputStruct->nodes[i][j].posX = stoi(stringVec[j+1]);
            outputStruct->nodes[i][j].posY = stoi(stringVec[j+2]);
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
    int i, j;

    set_draw_mode(DRAW_NORMAL);  // Should set this if your program does any XOR drawing in callbacks.
    clearscreen();  /* Should precede drawing for all drawscreens */

    // Draw grid on screen
    setlinestyle(SOLID);
    setlinewidth(0);
    setcolor(GRID_COLOR);
    for(i = 0; i < grid.gridSizeX; i++)
    {
        for(j = 0; j < grid.gridSizeY; j++)
        {
            drawrect(
                (float) (GRID_PADDING_X + i*GRID_SIZE_X), (float) (GRID_PADDING_Y + j*GRID_SIZE_Y),
                (float) (GRID_PADDING_X + (i+1)*GRID_SIZE_X), (float) (GRID_PADDING_Y + (j+1)*GRID_SIZE_Y)
            );
        }
    }

    // Draw obstructions
    setcolor(OBSTRUCTION_COLOR);
    for(i = 0; i < grid.numObstructedCells; i++)
    {
        fillrect(
            (float)(GRID_PADDING_X + grid.obstructedX[i] * GRID_SIZE_X), (float)(GRID_PADDING_Y + grid.obstructedY[i] * GRID_SIZE_Y),
            (float)(GRID_PADDING_X + (grid.obstructedX[i]+1) * GRID_SIZE_X), (float)(GRID_PADDING_Y + (grid.obstructedY[i]+1) * GRID_SIZE_Y)
        );
    }

    // Draw net sources and sinks

    /* redrawing routine for still pictures.  Graphics package calls  *
     * this routine to do redrawing after the user changes the window *
     * in any way.                                      
     */

    //t_point polypts[3] = {{500.,400.},{450.,480.},{550.,480.}};
    //t_point polypts2[4] = {{700.,400.},{650.,480.},{750.,480.}, {800.,400.}};

    //setfontsize(10);
    //setlinestyle(SOLID);
    //setlinewidth(1);
    //setcolor(BLACK);

    //drawtext(110., 55., "colors", 150.);
    //setcolor(LIGHTGREY);
    //fillrect(150., 30., 200., 80.);
    //setcolor(DARKGREY);
    //fillrect(200., 30., 250., 80.);
    //setcolor(WHITE);
    //fillrect(250., 30., 300., 80.);
    //setcolor(BLACK);
    //fillrect(300., 30., 350., 80.);
    //setcolor(BLUE);
    //fillrect(350., 30., 400., 80.);
    //setcolor(GREEN);
    //fillrect(400., 30., 450., 80.);
    //setcolor(YELLOW);
    //fillrect(450., 30., 500., 80.);
    //setcolor(CYAN);
    //fillrect(500., 30., 550., 80.);
    //setcolor(RED);
    //fillrect(550., 30., 600., 80.);
    //setcolor(DARKGREEN);
    //fillrect(600., 30., 650., 80.);
    //setcolor(MAGENTA);
    //fillrect(650., 30., 700., 80.);
    //setcolor(WHITE);
    //drawtext(400., 55., "fillrect", 150.);

    //setcolor(BLACK);
    //drawtext(250., 150., "drawline", 150.);
    //setlinestyle(SOLID);
    //drawline(200., 120., 200., 200.);
    //setlinestyle(DASHED);
    //drawline(300., 120., 300., 200.);

    //setcolor(MAGENTA);
    //drawtext(450, 160, "drawellipticarc", 150);
    //drawellipticarc(550, 160, 30, 60, 90, 270);
    //drawtext(720, 160, "fillellipticarc", 150);
    //fillellipticarc(800, 160, 30, 60, 90, 270);

    //setcolor(BLUE);
    //drawtext(190., 300., "drawarc", 150.);
    //drawarc(190., 300., 50., 0., 270.);
    //drawarc(300., 300., 50., 0., -180.);
    //fillarc(410., 300., 50., 90., -90.);
    //fillarc(520., 300., 50., 0., 360.);
    //setcolor(BLACK);
    //drawtext(520., 300., "fillarc", 150.);
    //setcolor(BLUE);
    //fillarc(630., 300., 50., 90., 180.);
    //fillarc(740., 300., 50., 90., 270.);
    //fillarc(850., 300., 50., 90., 30.);
    //setcolor(RED);
    //fillpoly(polypts, 3);
    //fillpoly(polypts2, 4);
    //setcolor(BLACK);
    //drawtext(500., 450., "fillpoly", 150.);
    //setcolor(DARKGREEN);
    //drawtext(500., 610., "drawrect", 150.);
    //drawrect(350., 550., 650., 670.);


    //setcolor(BLACK);
    //setfontsize(8);
    //drawtext(100., 770., "8 Point Text", 800.);
    //setfontsize(12);
    //drawtext(400., 770., "12 Point Text", 800.);
    //setfontsize(15);
    //drawtext(700., 770., "18 Point Text", 800.);
    //setfontsize(24);
    //drawtext(300., 830., "24 Point Text", 800.);
    //setfontsize(32);
    //drawtext(700., 830., "32 Point Text", 800.);
    //setfontsize(10);

    //setlinestyle(SOLID);
    //drawtext(200., 900., "Thin line (width 1)", 800.);
    //setlinewidth(1);
    //drawline(100., 920., 300., 920.);
    //drawtext(500., 900., "Width 3 Line", 800.);
    //setlinewidth(3);
    //drawline(400., 920., 600., 920.);
    //drawtext(800., 900., "Width 6 Line", 800.);
    //setlinewidth(6);
    //drawline(700., 920., 900., 920.);

    //setlinewidth(1);
    //setcolor(GREEN);


    //if(have_entered_line)
    //	drawline(x1, y1, x2, y2);

    // Screen redraw will get rid of a rubber line.  
    //have_rubber_line = false;
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
