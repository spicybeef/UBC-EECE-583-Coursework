#pragma once

#include <fstream>
#include <vector>
#include <utility>
#include "graphics.h"

// Constants used in drawing

#define GRID_COLOR				DARKGREY
#define OBSTRUCTION_COLOR		LIGHTGREY
#define MAX_NET_COLORS          8

// Constants used in the algorithm
#define MAXIMUM_ROUTING_RETRIES 200

// This enum contains the Lee Moore routing algorithm's state
typedef enum
{
    STATE_LP_IDLE = 0,
    STATE_LP_SEEK,
    STATE_LP_EXPANSION,
    STATE_LP_ROUTE_SUCCESS,
    STATE_LP_ROUTE_FAILURE
} routingState_e;

// This enum contains property information about a cell
typedef enum
{
    CELL_EMPTY = 0,
    CELL_OBSTRUCTED,
    CELL_NET_NODE_UNCONN,
    CELL_NET_NODE_CONN,
    CELL_NET_WIRE_UNCONN,
    CELL_NET_WIRE_CONN
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

// Macros to help index the current direction we are in
typedef enum
{
    DIR_IDX_NS_Y = 0,
    DIR_IDX_EW_X,
    DIR_IDX_NUM
} directionIndex_e;


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
    // Input file storage
    unsigned int                            gridSizeX;          ///< The grid size in X
    unsigned int                            gridSizeY;          ///< The grid size in Y

    std::vector<posStruct_t>                obstructions;       ///< This contains all of the obstructions
    std::vector<std::vector<posStruct_t>>   nodes;              ///< This contains all of the nets and their sources and sinks
} parsedInputStruct_t;

typedef struct
{
    // Routing variables
    unsigned int                                            currentNet;         ///< The current net being routed
    std::vector<cellStruct_t*>                              currentNodes;       ///< a list of the net's current nodes
    std::vector<std::pair<cellStruct_t*, cellStruct_t*>>    currentEdges;       ///< Store the edges for the current net connections
    unsigned int                                            currentNode;        ///< The current net's node being sought
    cellStruct_t                                            *currentNodePointer;///< A pointer to the current node

    cardinalDir_e                                           nextNodeDir[2];     ///< The direction of the next node
    directionIndex_e                                        directionIndex;     ///< The index of the current direction we are routing
    cellStruct_t                                            *nextNodePointer;   ///< A pointer to the next node
    
    int                                                     cellsLeftX;         ///< Cells left to target X
    int                                                     cellsLeftY;         ///< Cells left to target Y

    std::vector<unsigned int>                               netRoutedNodes;     ///< A counter for every net to keep track of how many nodes left to route
    cellStruct_t                                            *lastCell;          ///< A pointer for the last cell to walk back from
    int                                                     currentExpansion;   ///< The current expansion layer
    std::vector<std::vector<cellStruct_t*>>                 expansionList;      ///< A list containing the cells in the current expansion layer
    std::vector<cellStruct_t*>                              lastRoute;          ///< Keep a list of the last route in case we need to route to additional sinks
    routingState_e                                          currentRoutingState;///< The current routing state
    unsigned int                                            currentRetries;     ///< A counter for additional attempts to route a grid

    // Grid cell properties
    std::vector<std::vector<cellStruct_t>>                  cells;              ///< These are the cells that make up the routing grid

} gridStruct_t;

void DrawScreen(void);
void DrawCell(cellStruct_t *cell);

// LeeMoore Algorithm
void LineProbeInit(parsedInputStruct_t *parsedInputStruct, gridStruct_t *gridStruct);
void LineProbeExec(parsedInputStruct_t *parsedInputStruct, gridStruct_t *gridStruct, stepType_e stepType);

// Helpers
int MyRandomInt(int i);
void GetDirection(cellStruct_t *cell0, cellStruct_t *cell1, gridStruct_t *gridStruct);
void GetDistanceDelta(cellStruct_t *cell0, cellStruct_t *cell1, unsigned int * distanceDelta);
void ResetCellExpansion(gridStruct_t *gridStruct);
bool ParseInputFile(std::ifstream *inputFile, parsedInputStruct_t *inputStruct);
bool PopulateCellInfo(parsedInputStruct_t *parsedInputStruct, gridStruct_t *gridStruct);

void ActOnNewButtonFunc(void(*drawscreen_ptr)(void));
void ActOnButtonPress(float x, float y);
void ActOnMouseMove(float x, float y);
void ActOnKeyPress(char c);

std::vector<std::string> SplitString(std::string inString, char delimiter);
bool ParseInputFile(std::ifstream *inputFile, gridStruct_t *gridStruct);
bool PopulateCellInfo(gridStruct_t *gridStruct);