#pragma once

#include <SFML/Graphics.hpp>

// typedef helpers to make things legible
typedef std::vector<std::vector<unsigned int>>  netVec;

// Forward declarations since net and cell structs reference each other
typedef struct Net netStruct_t;
typedef struct Node nodeStruct_t;

// Program mode
typedef enum
{
    PROGRAM_MODE_CLI = 0,       /// Program runs in command line only
    PROGRAM_MODE_GUI,           /// Program runs with full graphics
    PROGRAM_MODE_TEST,          /// Program runs its built-in self test
    PROGRAM_MODE_NUM
} programMode_e;

// Draw update mode enum
typedef enum
{
    DRAW_EVERY_TICK = 0,        /// Graphics draw every iteration of the drawing sequence
    DRAW_EVERY_NODE_PERCENT,    /// Graphics draw after running the partitioning for N iterations where N is a percentage of the total nodes in a benchmark
    DRAW_EVERY_PERIOD,          /// Graphics draw after a period of time
    DRAW_MODE_NUM
} drawUpdateMode_e;

// Partitioning states enum
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

// Net segment struct
typedef struct
{
    unsigned int                                net;                    ///< Net segment's net id
    std::vector<unsigned int>                   nodes;                  ///< Nodes on either side of the segment
} netSegmentStruct_t;

// Net struct
typedef struct Net
{
    unsigned int                                id;                     ///< Net's id
    std::vector<unsigned int>                   nodes;                  ///< Pointers to the net's nodes
    std::vector<unsigned int>                   segments;               ///< Individual segments of the net
    sf::Color                                   color;                  ///< Net color
} netStruct_t;

// Node struct
typedef struct Node
{
    posStruct_t                                 pos;                    ///< Current position of the node
    drawPosStruct_t                             drawPos;                ///< Current drawing position of the node's center

    nodeState_e                                 state;                  ///< Current node state
    int                                         gain;                   ///< Current node gain

    std::vector<unsigned int>                   neighbors;              ///< Pointers to the node's neighbors
    std::vector<unsigned int>                   segments;               ///< Segment indices to the node's connected segments
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