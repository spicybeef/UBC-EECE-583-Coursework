#pragma once

#include <vector>

// Program defaults
// Graphics constants
#define WIN_VIEWPORT_WIDTH                      800.f
#define WIN_VIEWPORT_HEIGHT                     600.f
#define WIN_INFOPORT_WIDTH                      WIN_VIEWPORT_WIDTH
#define WIN_INFOPORT_HEIGHT                     100.f
#define WIN_GRAPHICPORT_WIDTH                   WIN_VIEWPORT_WIDTH
#define WIN_GRAPHICPORT_HEIGHT                  (WIN_VIEWPORT_HEIGHT - WIN_INFOPORT_HEIGHT)
#define WIN_INFOPORT_PADDING                    10.f
// Grid constants
#define GRID_SHRINK_FACTOR                      0.6f

typedef struct
{
    unsigned int                                row;            ///< Cell row
    unsigned int                                col;            ///< Cell column

} posStruct_t;

typedef struct Cell
{
    unsigned int                                id;             ///< Block ID
    posStruct_t                                 pos;            ///< Current position of the cell

} cellStruct_t;

typedef struct Net
{
    std::vector<Cell*>                          connections;    ///< Pointers to the cell's connections
    unsigned int                                halfPerim;      ///< Current half perimeter of the net

} netStruct_t;

typedef struct
{
    unsigned int                                numRows;        ///< Number of parsed rows
    unsigned int                                numCols;        ///< Number of parsed columns

    unsigned int                                numCells;       ///< Number of cells to place
    unsigned int                                numConnections; ///< The number of connections

    std::vector<std::vector<unsigned int>>      nets;           ///< Parsed nets

} parsedInputStruct_t;

typedef struct
{
    std::vector<cellStruct_t>                   cells;          ///< Cells
    std::vector<netStruct_t>                    nets;           ///< Nets

} placerStruct_t;

// Helpers
bool parseInputFile(std::ifstream *inputFile, parsedInputStruct_t *inputStruct);
sf::View calcView(const sf::Vector2u &windowsize, const sf::Vector2u &designedsize);
std::vector<sf::RectangleShape> generateGrid(parsedInputStruct_t *input);
std::vector<std::string> splitString(std::string inString, char delimiter);