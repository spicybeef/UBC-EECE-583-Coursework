#pragma once

#include <fstream>

// Constants used in router
#define NULL32					0xDEADBEEF
#define MAX_GRID_SIZE_X			256
#define MAX_GRID_SIZE_Y			256
#define MAX_OBSTRUCTED_CELLS	MAX_GRID_SIZE_X * MAX_GRID_SIZE_Y
#define MAX_NETS				32
#define MAX_NODES				32

typedef struct
{
	int posX;
	int posY;
	void *nextNode;

} netNodeStruct_t;

typedef struct
{
	int gridSizeX;
	int gridSizeY;

	int numObstructedCells;
	int obstructedX[MAX_OBSTRUCTED_CELLS];
	int obstructedY[MAX_OBSTRUCTED_CELLS];

	int numNets;
	netNodeStruct_t nodes[MAX_NETS][MAX_NODES];

} gridStruct_t;

extern gridStruct_t Grid;

void Delay(void);
void DrawScreen(void);
void ActOnNewButtonFunc(void(*drawscreen_ptr)(void));
void ActOnButtonPress(float x, float y);
void ActOnMouseMove(float x, float y);
void ActOnKeyPress(char c);

std::vector<std::string> SplitString(std::string inString, char delimiter);
bool ParseGridStruct(std::ifstream *inputFile, gridStruct_t *outputStruct);