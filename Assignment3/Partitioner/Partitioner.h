#pragma once

// C++ Includes
#include <vector>
#include <queue>

// SFML Includes
#include <SFML/Graphics.hpp>

// Program defaults
// Graphics constants
#define WIN_VIEWPORT_WIDTH                      1000.f
#define WIN_VIEWPORT_HEIGHT                     800.f
#define WIN_INFOPORT_WIDTH                      WIN_VIEWPORT_WIDTH
#define WIN_INFOPORT_HEIGHT                     100.f
#define WIN_GRAPHICPORT_WIDTH                   WIN_VIEWPORT_WIDTH
#define WIN_GRAPHICPORT_HEIGHT                  (WIN_VIEWPORT_HEIGHT - WIN_INFOPORT_HEIGHT)
#define WIN_INFOPORT_PADDING                    10.f
// Grid constants
#define GRID_SHRINK_FACTOR                      0.8f
#define CELL_SHRINK_FACTOR						0.7f
// typedef helpers to make things legible
typedef std::vector<std::vector<unsigned int>>	netVec;

// partitioning states enum
typedef enum
{
    STATE_START = 0,
    STATE_PARTITIONING,
    STATE_FINISHED,
    STATE_NUM
} state_e;

// partitioning temperature decrease type
typedef enum
{
    TEMP_DECREASE_LINEAR = 0,
    TEMP_DECREASE_EXP,
    TEMP_DECREASE_NUM
} temperatureDecrease_e;

// Dimensions enum
typedef enum
{
    DIM_HORIZONTAL = 0,
    DIM_VERTICAL
} dimension_e;

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
	sf::Color									color;				    ///< Net color
} netStruct_t;

// Cell struct
typedef struct Node
{
    unsigned int                                id;                     ///< Block ID
    posStruct_t                                 pos;                    ///< Current position of the cell
    drawPosStruct_t                             drawPos;                ///< Current drawing position of the cell's center
    netStruct_t                                 *nodeNet;               ///< A pointer to the cell's net, for easy reference
	std::vector<nodeStruct_t*>					neighbours;				///< Pointers to the cell's neighbours
} nodeStruct_t;

// Parsed input struct
typedef struct
{
    unsigned int                                numRows;                ///< Number of parsed rows
    unsigned int                                numCols;                ///< Number of parsed columns

    unsigned int                                numNodes;               ///< Number of nodes to place
    unsigned int                                numConnections;         ///< The number of connections

	netVec                                      nets;                   ///< Parsed nets

} parsedInputStruct_t;

typedef struct
{
	unsigned int                                numRows;                ///< Number of rows
	unsigned int                                numCols;                ///< Number of columns

	float                                       cellSize;               ///< Current cellsize
	float                                       cellOffset;             ///< Cell offset for maximized dimension
	float                                       cellOppositeOffset;     ///< Cell offset for other dimension
	dimension_e                                 maximizedDim;           ///< Current maximized dimension
} cellPropertiesStruct_t;

// Constants for the partitioner
typedef struct
{
    char                                        *filename;              ///< Current filename

	cellPropertiesStruct_t						cellProperties;			///< Grid cell properties

    std::vector<std::vector<nodeStruct_t*>>     grid;                   ///< Grid containing pointers to nodes
    std::vector<nodeStruct_t>                   nodes;                  ///< Nodes
    std::vector<netStruct_t>                    nets;                   ///< Nets

	clock_t										starttime;				///< start time for annealing
	clock_t										endtime;				///< end time for annealing
    state_e                                     currentState;           ///< Current partitioning state

} partitionerStruct_t;

void doPartitioning(partitionerStruct_t *partitionerStruct);

// Input file parser
bool parseInputFile(std::ifstream *inputFile, parsedInputStruct_t *inputStruct);

drawPosStruct_t getGridCellCoordinate(cellPropertiesStruct_t cellProperties, unsigned int col, unsigned int row);
void updateNodePosition(cellPropertiesStruct_t cellProperties, nodeStruct_t &node, std::vector<std::vector<nodeStruct_t*>> &grid, unsigned int col, unsigned int row);
void generateNodeConnections(parsedInputStruct_t *inputStruct, partitionerStruct_t *partitionerStruct);

void generateNodes(unsigned int totalNodes, unsigned int numNodes, partitionerStruct_t *partitionerStruct);
void generateNodePlacement(unsigned int numCols, unsigned int numRows, cellPropertiesStruct_t cellProperties);

std::vector<std::vector<nodeStruct_t*>> initializeGridModel(unsigned int numCols, unsigned int numRows, partitionerStruct_t *partitionerStruct);
void initializeNodeNet(std::vector<netStruct_t> &nets);
void initializeNetColors(std::vector<netStruct_t> &nets, unsigned int col, unsigned int row);
void updateNetColor(nodeStruct_t &node);

// These functions create the SFML primitives used to draw the partitioner
std::vector<sf::Vertex> generateNetGeometries(std::vector<netStruct_t> &nets);
std::vector<sf::RectangleShape> generatePlacedNodeGeometries(std::vector<nodeStruct_t> &cells, dimension_e maximizedDim, float cellSize, float cellOffset, float cellOppositeOffset, state_e state);
std::vector<sf::RectangleShape> generateGridGeometries(parsedInputStruct_t *inputStruct, partitionerStruct_t *partitionerStruct);

// Utility functions
int getRandomInt(int i);
double getRandomDouble(void);
std::vector<std::string> splitString(std::string inString, char delimiter);
sf::View calcView(const sf::Vector2u &windowsize, const sf::Vector2u &designedsize);
double calculateStandardDeviation(std::vector<int> dataSet);
std::string getInfoportString(partitionerStruct_t *partitionerStruct);
