#pragma once

// C++ Includes
#include <vector>

// SFML Includes
#include <SFML/Graphics.hpp>

// Program Includes
#include "Partitioner.h"

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

// Col/row position struct
typedef struct
{
    unsigned int                                row;                    ///< Cell row
    unsigned int                                col;                    ///< Cell column
} posStruct_t;

// Drawing x/y position struct
typedef struct
{
    float                                       x;                      ///< X coordinate
    float                                       y;                      ///< Y coordinate
} drawPosStruct_t;

// Forward declarations since net and cell structs reference each other
typedef struct Net netStruct_t;
typedef struct Node nodeStruct_t;

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

typedef struct
{
    float                                       cellSize;               ///< Current cellsize
    float                                       cellOffset;             ///< Cell offset for maximized dimension
    float                                       cellOppositeOffset;     ///< Cell offset for other dimension

    dimension_e                                 maximizedDim;           ///< Current maximized dimension

    unsigned int                                divider;                ///< Column or row that starts the partition

} cellPropertiesStruct_t;

// typedef helpers to make things legible
typedef std::vector<nodeStruct_t*> gridRowVec;
typedef std::vector<gridRowVec> gridColVec;
typedef gridColVec gridVec;

class NetList
{
public:
    //** Constructor / destructor
    NetList(parsedInputStruct_t parsedInput);
    ~NetList();

    //** Class Functions
    // Initializers
private:
    void initializeGridModel();
    void initializeNodes();
    void initializeNets();
    void initializeNodeNeighbours();
    void initializeNodeNets();
    void initializeNetColors();
    void initializeCellProperties();

public:
    // These function create the SFML primitives used to draw the partitioner objects
    std::vector<sf::RectangleShape> generateGridGeometries();
    std::vector<sf::Vertex> generateNetGeometries();
    std::vector<sf::RectangleShape> generatePlacedNodeGeometries();
    std::vector<sf::Text> generatePlacedNodeText();
    std::vector<sf::Vertex> generatePartitionerDivider();

    // Node position functions
    void randomizeNodePlacement();
    void swapNodePartition(unsigned int id);
    void getNodePosition(unsigned int id, unsigned int *col, unsigned int *row);
    void updateNodePosition(unsigned int id, unsigned int col, unsigned int row);

    // Algorithmic helpers
    void lockNode(unsigned int id);
    void unlockNode(unsigned int id);
    void unlockAllNodes();
    int calculateTotalCost();
    int calculateNodeGain(unsigned int id);
    void updateAllNodeGains();

private:
    // These function are helpers
    bool doesSegmentCrossDivider(std::vector<sf::Vector2f> segment);
    std::vector<sf::Vector2f> getDividerVector();
    nodeStruct_t * getNodePointer(unsigned int id);
    void updateNetColor(unsigned int id);
    drawPosStruct_t getGridCellCoordinate(unsigned int col, unsigned int row);

    //** Class Member Variables
private:
    gridVec mGrid;                          ///< Grid containing pointers to nodes
    unsigned int mNumCols;                  ///< Number of grid columns
    unsigned int mNumRows;                  ///< Number of grid rows
    std::vector<nodeStruct_t> mNodes;       ///< Nodes
    std::vector<netStruct_t> mNets;         ///< Nets
    cellPropertiesStruct_t mCellProperties; ///< Grid cell properties
    parsedInputStruct_t mParsedInput;       ///< Struct containing the parsed input from the file

    sf::Font mFont;                         ///< Store a copy of the font we'll be using
};

