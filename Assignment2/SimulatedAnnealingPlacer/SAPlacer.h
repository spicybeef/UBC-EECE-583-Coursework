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
// Infoport constants
#define INFOPORT_CHAR_WIDTH                     109
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
    unsigned int                                row;            ///< Cell row
    unsigned int                                col;            ///< Cell column
} posStruct_t;

// Drawing x/y position struct
typedef struct
{
    float                                       x;              ///< X coordinate
    float                                       y;              ///< Y coordinate
} drawPosStruct_t;

// Cell struct
typedef struct Cell
{
    unsigned int                                id;             ///< Block ID
    posStruct_t                                 pos;            ///< Current position of the cell
    drawPosStruct_t                             drawPos;        ///< Current drawing position of the cell's center
} cellStruct_t;

// Net struct
typedef struct Net
{
    std::vector<Cell*>                          connections;        ///< Pointers to the cell's connections
    unsigned int                                halfPerim;          ///< Current half perimeter of the net
	sf::Color									color;				///< Net color
} netStruct_t;

// Parsed input struct
typedef struct
{
    unsigned int                                numRows;            ///< Number of parsed rows
    unsigned int                                numCols;            ///< Number of parsed columns

    unsigned int                                numCells;           ///< Number of cells to place
    unsigned int                                numConnections;     ///< The number of connections

	netVec                                      nets;               ///< Parsed nets

} parsedInputStruct_t;

// Constants for simulated annealing
#define ACCEPTANCE_RATE_WINDOW                  100                 ///< Window for the acceptance rate calculation
#define TEMP_LINEAR_COEFFICIENT                 0.75                ///< Temperature decrease linear coefficient

typedef struct
{
    float                                       cellSize;           ///< Current cellsize
    float                                       cellOffset;         ///< Cell offset for maximized dimension
    float                                       cellOppositeOffset; ///< Cell offset for other dimension
    dimension_e                                 maximizedDim;       ///< Current maximized dimension

    std::vector<std::vector<cellStruct_t*>>     grid;               ///< Grid containing pointers to cells
    std::vector<cellStruct_t>                   cells;              ///< Cells
    std::vector<netStruct_t>                    nets;               ///< Nets

    state_e                                     currentState;       ///< Current simulated annealing state
    unsigned int                                movesPerTempDec;    ///< Number of swaps to perform per temperature step
    unsigned int                                totalHalfPerim;     ///< Total half perimeter
    double                                      temperature;        ///< Current simulated annealing temperature
    std::vector<bool>                           acceptanceTracker;  ///< Keep track of what's been accepted
    double                                      acceptanceRate;     ///< Current acceptance rate
} placerStruct_t;

void doSimulatedAnnealing(placerStruct_t *placerStruct);

bool parseInputFile(std::ifstream *inputFile, parsedInputStruct_t *inputStruct);
int myRandomInt(int i);
drawPosStruct_t getGridCellCoordinate(placerStruct_t *placerStruct, unsigned int col, unsigned int row);
void updateCellPosition(placerStruct_t *placerStruct, cellStruct_t *cell, unsigned int col, unsigned int row);
void generateCellConnections(parsedInputStruct_t *inputStruct, placerStruct_t *placerStruct);
void generateGridModel(unsigned int numCols, unsigned int numRows, placerStruct_t *placerStruct);
void generateCells(unsigned int numCells, placerStruct_t *placerStruct);
void generateCellPlacement(unsigned int numCols, unsigned int numRows, placerStruct_t *placerStruct);
void swapCells(cellStruct_t *cell0, cellStruct_t *cell1, placerStruct_t *placerStruct);

std::vector<sf::Vertex> generateNetLines(placerStruct_t *placerStruct);
std::vector<sf::RectangleShape> generateGrid(parsedInputStruct_t *inputStruct, placerStruct_t *placerStruct);
std::vector<std::string> splitString(std::string inString, char delimiter);
sf::View calcView(const sf::Vector2u &windowsize, const sf::Vector2u &designedsize);

std::string getInfoportString(placerStruct_t *placerStruct);

double calculateStandardDeviation(std::vector<int> dataSet);
double calculateNewTemp(double oldTemp, double stdDev, temperatureDecrease_e mode);
float calculateAcceptanceRate(std::vector<bool> acceptanceTracker);
unsigned int calculateTotalHalfPerim(std::vector<netStruct_t> &nets);