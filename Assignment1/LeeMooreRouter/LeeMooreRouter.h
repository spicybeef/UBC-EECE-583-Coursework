#pragma once

#include <fstream>
#include <vector>
#include "graphics.h"

// Constants used in drawing

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

// This enum is a counter incrementing for each cardinal direction
typedef enum
{
    DIR_NORTH = 0,
    DIR_EAST,
    DIR_SOUTH,
    DIR_WEST,
    DIR_NUM
} cardinalDir_e;

// This struct stores an X, Y position
typedef struct
{
    unsigned int posX;
    unsigned int posY;

} posStruct_t;

// This struct contains a cell's properties
typedef struct Cell
{
    posStruct_t     coord;                  ///< Cell's current coordinates

    int             currentNet;             ///< This is the current routed net
    cellProp_e      currentCellProp;        ///< This is the current cell's property

    int             currentNumber;          ///< This is the current expansion number

    Cell*           neighbours[DIR_NUM];    ///< These are pointers to a cell's neighbours in each cardinal direction (indexed by cardinalDir_e)
} cellStruct_t;

typedef struct
{
    unsigned int gridSizeX;                                 ///< The grid size in X
    unsigned int gridSizeY;                                 ///< The grid size in Y

    // Input file storage
    std::vector<posStruct_t> obstructions;                  ///< This contains all of the obstructions
    std::vector<std::vector<posStruct_t>> nodes;            ///< This contains all of the nets and their sources and sinks

    // Routing state
    unsigned int    currentNet;                             ///< The current net being routed
    int             currentExpansion;                       ///< The current expansion layer
    std::vector<std::vector<cellStruct_t*>> expansionList;  ///< A list containing the cells in the current expansion layer
    routingState_e  currentRoutingState;                    ///< The current routing state

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