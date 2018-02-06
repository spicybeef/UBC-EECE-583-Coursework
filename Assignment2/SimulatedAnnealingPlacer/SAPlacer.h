#pragma once

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

bool ParseInputFile(std::ifstream *inputFile, parsedInputStruct_t *inputStruct);
