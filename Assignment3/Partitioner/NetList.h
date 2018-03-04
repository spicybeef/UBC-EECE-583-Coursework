#pragma once

// C++ Includes
#include <vector>

// SFML Includes
#include <SFML/Graphics.hpp>

#include "Types.h"

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
    void initializeNetSegments();
    void initializeNodeNeighbours();
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
    unsigned int getNodePartition(unsigned int id);
    posStruct_t getNodePosition(unsigned int id);
    void updateNodePosition(unsigned int id, posStruct_t pos);

    // Algorithmic helpers
    void lockNode(unsigned int id);
    void unlockNode(unsigned int id);
    void unlockAllNodes();
    bool isNodeLocked(unsigned int id);
    unsigned int calculateCurrentCutSize();
    int calculateNodeGain(unsigned int id);
    int getNodeGain(unsigned int id);
    void updateAllNodeGains();
    unsigned int getNumNodes();
    unsigned int getNumCols();
    unsigned int getNumRows();
    unsigned int getNumNets();

private:
    // These function are helpers
    bool doesSegmentCrossDivider(std::vector<sf::Vector2f> segment);
    std::vector<sf::Vector2f> getDividerVector();
    nodeStruct_t * getNodePointer(unsigned int id);
    netStruct_t * getNetPointer(unsigned int id);
    netSegmentStruct_t * getNetSegmentPointer(unsigned int id);
    void updateNetColor(unsigned int id);
    drawPosStruct_t getGridCellCoordinate(unsigned int col, unsigned int row);

    //** Class Member Variables
private:
    gridVec mGrid;                                  ///< Grid containing pointers to nodes
    unsigned int mNumCols;                          ///< Number of grid columns
    unsigned int mNumRows;                          ///< Number of grid rows
    std::vector<nodeStruct_t> mNodes;               ///< Nodes
    std::vector<netStruct_t> mNets;                 ///< Nets
    std::vector<netSegmentStruct_t> mNetSegments;   ///< Net segments
    cellPropertiesStruct_t mCellProperties;         ///< Grid cell properties
    parsedInputStruct_t mParsedInput;               ///< Struct containing the parsed input from the file

    sf::Font mFont;                         ///< Store a copy of the font we'll be using
};

