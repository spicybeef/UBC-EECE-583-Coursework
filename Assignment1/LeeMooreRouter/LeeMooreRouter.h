#pragma once

#include <fstream>
#include "graphics.h"

// Constants used in router
#define NULL32					0xDEADBEEF
#define MAX_GRID_SIZE_X			256
#define MAX_GRID_SIZE_Y			256
#define MAX_OBSTRUCTED_CELLS	MAX_GRID_SIZE_X * MAX_GRID_SIZE_Y
#define MAX_NETS				32
#define MAX_NODES				32

// Constants used in drawing
#define GRID_PADDING_X			0
#define GRID_PADDING_Y			100
#define GRID_SIZE_X				30
#define GRID_SIZE_Y				30

#define GRID_COLOR				DARKGREY
#define OBSTRUCTION_COLOR		BLUE

typedef struct
{
    int posX;
    int posY;

} netNodeStruct_t;

typedef struct
{
    int gridSizeX;
    int gridSizeY;

    int numObstructedCells;
    int obstructedX[MAX_OBSTRUCTED_CELLS];
    int obstructedY[MAX_OBSTRUCTED_CELLS];

    int numNets;
    int nodesPerNet[MAX_NETS];
    netNodeStruct_t nodes[MAX_NETS][MAX_NODES];

} gridStruct_t;

void Delay(void);
void DrawScreen(void);
void ActOnNewButtonFunc(void(*drawscreen_ptr)(void));
void ActOnButtonPress(float x, float y);
void ActOnMouseMove(float x, float y);
void ActOnKeyPress(char c);

std::vector<std::string> SplitString(std::string inString, char delimiter);
bool ParseGridStruct(std::ifstream *inputFile, gridStruct_t *outputStruct);