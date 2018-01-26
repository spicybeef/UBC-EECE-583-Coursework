#pragma once

#include <fstream>
#include <vector>
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

#define MAX_NET_COLORS          8

// This enum contains the Lee Moore routing algorithm's state
typedef enum
{
    STATE_LM_IDLE = 0,
    STATE_LM_EXPANSION,
    STATE_LM_WALKBACK,
    STATE_LM_ROUTE_SUCCESS,
    STATE_LM_ROUTE_FAILURE
} routingState_e;

// This enum contains property information about a cell
typedef enum
{
    CELL_EMPTY = 0,
    CELL_OBSTRUCTED,
    CELL_NET_SOURCE,
    CELL_NET_SINK,
    CELL_NET_WIRE
} cellProp_e;

// This enum defines how large of a step to take in the algorithm
typedef enum
{
    STEP_SINGLE = 0,    ///< Step only a single step
    STEP_NET,           ///< Step an entire net (route a net and go to the next one)
    STEP_COMPLETE       ///< Attempt to route the entire grid
} stepType_e;

// This struct stores an X, Y position
typedef struct
{
    unsigned int posX;
    unsigned int posY;

} posStruct_t;

// This struct contains a cell's properties
typedef struct Cell
{
    posStruct_t     coord;          ///< Cell's current coordinates

    int             currentNet;     ///< This is the current routed net
    cellProp_e      currentCellProp;///< This is the current cell's property

    int             currentNumber;  ///< This is the current expansion number

    Cell*           norNeigh;       ///< This is a pointer to the cell's northern neighbour
    Cell*           easNeigh;       ///< This is a pointer to the cell's eastern neighbour
    Cell*           souNeigh;       ///< This is a pointer to the cell's southern neighbour
    Cell*           wesNeigh;       ///< This is a pointer to the cell's western neighbour
} cellStruct_t;

typedef struct
{
    unsigned int gridSizeX;                         ///< The grid size in X
    unsigned int gridSizeY;                         ///< The grid size in Y

    // Input file storage
    std::vector<posStruct_t> obstructions;          ///< This contains all of the obstructions
    std::vector<std::vector<posStruct_t>> nodes;    ///< This contains all of the nets and their sources and sinks

    // Routing state
    unsigned int    currentNet;                     ///< The current net being routed
    routingState_e  currentRoutingState;            ///< The current routing state

    // Grid cell properties
    std::vector<std::vector<cellStruct_t>> cells;   ///< These are the cells that make up the routing grid

} gridStruct_t;

void Delay(void);

void DrawScreen(void);
void DrawCell(cellStruct_t *cell);

void LeeMooreInit(gridStruct_t *gridStruct);
void LeeMooreExec(gridStruct_t *gridStruct, stepType_e stepType);

void ActOnNewButtonFunc(void(*drawscreen_ptr)(void));
void ActOnButtonPress(float x, float y);
void ActOnMouseMove(float x, float y);
void ActOnKeyPress(char c);

std::vector<std::string> SplitString(std::string inString, char delimiter);
bool ParseInputFile(std::ifstream *inputFile, gridStruct_t *gridStruct);
bool PopulateCellInfo(gridStruct_t *gridStruct);