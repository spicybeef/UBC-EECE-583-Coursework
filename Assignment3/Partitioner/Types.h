#pragma once

#include <SFML/Graphics.hpp>

// typedef helpers to make things legible
typedef std::vector<std::vector<unsigned int>>  netVec;

// Forward declarations since net and cell structs reference each other
typedef struct Net netStruct_t;
typedef struct Node nodeStruct_t;

// partitioning states enum
typedef enum
{
    STATE_INIT = 0,
    STATE_PARTITIONING_START,
    STATE_PARTITIONING_FIND_SWAP_CANDIDATES,
    STATE_PARTITIONING_SWAP_AND_LOCK,
    STATE_FINISHED,
    STATE_NUM
} state_e;

// Dimensions enum
typedef enum
{
    DIM_HORIZONTAL = 0,
    DIM_VERTICAL
} dimension_e;

// Node state
typedef enum
{
    NODE_STATE_UNLOCKED = 0,
    NODE_STATE_LOCKED
} nodeState_e;

// Parsed input struct
typedef struct
{
    unsigned int                                numRows;                ///< Number of parsed rows
    unsigned int                                numCols;                ///< Number of parsed columns

    unsigned int                                numNodes;               ///< Number of nodes to place
    unsigned int                                numConnections;         ///< The number of connections

    netVec                                      nets;                   ///< Parsed nets

} parsedInputStruct_t;

// Col/row position struct
typedef struct
{
    unsigned int                                col;                    ///< Column
    unsigned int                                row;                    ///< Row
} posStruct_t;

// Drawing x/y position struct
typedef struct
{
    float                                       x;                      ///< X coordinate
    float                                       y;                      ///< Y coordinate
} drawPosStruct_t;

// Net struct
typedef struct Net
{
    std::vector<nodeStruct_t*>                  connections;            ///< Pointers to the cell's connections
    sf::Color                                   color;                  ///< Net color
} netStruct_t;

// Cell struct
typedef struct Node
{
    posStruct_t                                 pos;                    ///< Current position of the cell
    drawPosStruct_t                             drawPos;                ///< Current drawing position of the cell's center

    nodeState_e                                 state;                  ///< Current node state
    int                                         gain;                   ///< Current node gain

    netStruct_t                                 *nodeNet;               ///< A pointer to the cell's net, for easy reference
    std::vector<nodeStruct_t*>                  neighbors;             ///< Pointers to the cell's neighbors
} nodeStruct_t;

// Typedefs for grid
typedef std::vector<nodeStruct_t*> gridRowVec;
typedef std::vector<gridRowVec> gridColVec;
typedef gridColVec gridVec;

typedef struct
{
    float                                       cellSize;               ///< Current cellsize
    float                                       cellOffset;             ///< Cell offset for maximized dimension
    float                                       cellOppositeOffset;     ///< Cell offset for other dimension

    dimension_e                                 maximizedDim;           ///< Current maximized dimension

    unsigned int                                divider;                ///< Column or row that starts the partition

} cellPropertiesStruct_t;