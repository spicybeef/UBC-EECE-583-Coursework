#pragma once

// C++ Includes
#include <vector>

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

// Parsed input struct
typedef struct
{
    unsigned int                                numRows;                ///< Number of parsed rows
    unsigned int                                numCols;                ///< Number of parsed columns

    unsigned int                                numNodes;               ///< Number of nodes to place
    unsigned int                                numConnections;         ///< The number of connections

	netVec                                      nets;                   ///< Parsed nets

} parsedInputStruct_t;

// Constants for the partitioner
typedef struct
{
    std::string                                 filename;              ///< Current filename

	clock_t										starttime;				///< start time for annealing
	clock_t										endtime;				///< end time for annealing
    state_e                                     currentState;           ///< Current partitioning state

} partitionerStruct_t;

void doPartitioning(partitionerStruct_t *partitionerStruct);

// Input file parser
bool parseInputFile(std::ifstream *inputFile, parsedInputStruct_t *inputStruct);

// Utility functions
int getRandomInt(int i);
double getRandomDouble(void);
std::vector<std::string> splitString(std::string inString, char delimiter);
sf::View calcView(const sf::Vector2u &windowsize, const sf::Vector2u &designedsize);
double calculateStandardDeviation(std::vector<int> dataSet);
std::string getInfoportString(partitionerStruct_t *partitionerStruct);
