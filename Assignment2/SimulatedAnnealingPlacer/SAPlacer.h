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
// typedef helpers to make things legible
typedef std::vector<std::vector<unsigned int>>	netVec;

// Simulated annealing states enum
typedef enum
{
    STATE_START = 0,
    STATE_ANNEALING,
    STATE_FINISHED,
    STATE_NUM
} state_e;

// Simulated annealing temperature decrease type
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
typedef struct Cell cellStruct_t;

// Net struct
typedef struct Net
{
    std::vector<cellStruct_t*>                  connections;            ///< Pointers to the cell's connections
    unsigned int                                maxHalfPerim;           ///< Maximum half perimeter of the net
	sf::Color									color;				    ///< Net color
} netStruct_t;

// Cell struct
typedef struct Cell
{
    unsigned int                                id;                     ///< Block ID
    posStruct_t                                 pos;                    ///< Current position of the cell
    drawPosStruct_t                             drawPos;                ///< Current drawing position of the cell's center
    netStruct_t                                 *cellNet;               ///< A pointer to the cell's net, for easy reference
} cellStruct_t;

// Parsed input struct
typedef struct
{
    unsigned int                                numRows;                ///< Number of parsed rows
    unsigned int                                numCols;                ///< Number of parsed columns

    unsigned int                                numCells;               ///< Number of cells to place
    unsigned int                                numConnections;         ///< The number of connections

	netVec                                      nets;                   ///< Parsed nets

} parsedInputStruct_t;

// Constants for simulated annealing
#define TEMP_LINEAR_COEFFICIENT                 0.9                     ///< Temperature decrease linear coefficient

typedef struct
{
    float                                       cellSize;               ///< Current cellsize
    float                                       cellOffset;             ///< Cell offset for maximized dimension
    float                                       cellOppositeOffset;     ///< Cell offset for other dimension
    dimension_e                                 maximizedDim;           ///< Current maximized dimension

    std::vector<std::vector<cellStruct_t*>>     grid;                   ///< Grid containing pointers to cells
    std::vector<cellStruct_t>                   cells;                  ///< Cells
    std::vector<netStruct_t>                    nets;                   ///< Nets

    state_e                                     currentState;           ///< Current simulated annealing state
    unsigned int                                movesPerTempDec;        ///< Number of swaps to perform per temperature step
    unsigned int                                currentMove;            ///< Current move
    unsigned int                                currentHalfPerimSum;    ///< Current half perimeter sum
    unsigned int                                startingHalfPerimSum;   ///< Starting half perimeter sum
    double                                      temperature;            ///< Current simulated annealing temperature
    unsigned int                                totalTempDecrements;    ///< Count the number of temperature decrements so far
    std::vector<bool>                           acceptanceTracker;      ///< Keeps track of what's been accepted
    std::vector<int>                            costTracker;            ///< Keeps track of the cost
} placerStruct_t;

void doSimulatedAnnealing(placerStruct_t *placerStruct);

bool parseInputFile(std::ifstream *inputFile, parsedInputStruct_t *inputStruct);
int getRandomInt(int i);
double getRandomDouble(void);
drawPosStruct_t getGridCellCoordinate(placerStruct_t *placerStruct, unsigned int col, unsigned int row);
void updateCellPosition(placerStruct_t *placerStruct, cellStruct_t *cell, unsigned int col, unsigned int row);
void generateCellConnections(parsedInputStruct_t *inputStruct, placerStruct_t *placerStruct);
void generateGridModel(unsigned int numCols, unsigned int numRows, placerStruct_t *placerStruct);
void generateCells(unsigned int numCells, placerStruct_t *placerStruct);
void generateCellPlacement(unsigned int numCols, unsigned int numRows, placerStruct_t *placerStruct);
void updateCellNet(std::vector<netStruct_t> &nets);
void swapCells(cellStruct_t *cell0, cellStruct_t *cell1, placerStruct_t *placerStruct);
void initializeNetColors(std::vector<netStruct_t> &nets, unsigned int col, unsigned int row);
void updateNetColor(cellStruct_t &cell);

std::vector<sf::Vertex> generateNetLines(placerStruct_t *placerStruct);
std::vector<sf::RectangleShape> generateGrid(parsedInputStruct_t *inputStruct, placerStruct_t *placerStruct);
std::vector<std::string> splitString(std::string inString, char delimiter);
sf::View calcView(const sf::Vector2u &windowsize, const sf::Vector2u &designedsize);

std::string getInfoportString(placerStruct_t *placerStruct);

double calculateStandardDeviation(std::vector<int> dataSet);
double calculateNewTemp(double oldTemp, double stdDev, temperatureDecrease_e mode);
float calculateAcceptanceRate(std::vector<bool> acceptanceTracker);
unsigned int calculateHalfPerim(netStruct_t &net);
unsigned int calculateTotalHalfPerim(std::vector<netStruct_t> &nets);