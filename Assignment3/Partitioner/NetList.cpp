#include <algorithm>

#include "NetList.h"
#include "Util.h"
#include "main.h"

NetList::NetList(parsedInputStruct_t parsedInput)
{
    // Copy into the object the file's parsed input
    mParsedInput = parsedInput;

    // Fill out the number of rows and columns
    mNumCols = mParsedInput.numCols * 2;
    mNumRows = mParsedInput.numRows * 2;

    // Initialize internal structures now that we have a parsed input
    initializeGridModel();
    initializeNodes();
    initializeNets();
    initializeNodeNets();
    initializeNetColors();
    initializeCellProperties();
}

NetList::~NetList()
{
}

void NetList::initializeGridModel()
{
    unsigned int i, j;

    gridColVec col;
    gridRowVec row;

    mGrid.clear();

    // Generate the grid model
    for (i = 0; i < mNumCols; i++)
    {
        row.clear();
        for (j = 0; j < mNumRows; j++)
        {
            row.push_back(nullptr);
        }
        col.push_back(row);
    }

    mGrid = col;
}

void NetList::initializeNodes()
{
    unsigned int i;

    mNodes.clear();

    // For each cell
    for (i = 0; i < mParsedInput.numNodes; i++)
    {
        // Add a cell
        mNodes.push_back(nodeStruct_t());
        // Null the net pointer
        mNodes.back().nodeNet = nullptr;
    }
}

void NetList::initializeNets()
{
    unsigned int i, j;
    nodeStruct_t *cellPointer;

    // Clear any existing nets
    mNets.clear();

    // Go through the parsed input, and add each net
    for (i = 0; i < mParsedInput.nets.size(); i++)
    {
        // Push a fresh net to the placer
        mNets.push_back(netStruct_t());

        for (j = 0; j < mParsedInput.nets[i].size(); j++)
        {
            // Give the net its node references
            cellPointer = &mNodes[mParsedInput.nets[i][j]];
            mNets.back().connections.push_back(cellPointer);
        }
    }
}

void NetList::initializeNodeNeighbours()
{
    unsigned int i, j;

    // Go through the nets and its connections, and update all of the nodes' neighbours
    for (i = 0; i < mNets.size(); i++)
    {
        for (j = 0; j < mNets[i].connections.size() - 1; j++)
        {
            // Make these nodes neighbours
            mNets[i].connections[j]->neighbours.push_back(mNets[i].connections[j + 1]);
            mNets[i].connections[j + 1]->neighbours.push_back(mNets[i].connections[j]);
        }
    }
}

void NetList::initializeNodeNets()
{
    unsigned int i, j;

    // Go through the nets and its connections, and update all of the nodes' nets
    for (i = 0; i < mNets.size(); i++)
    {
        for (j = 0; j < mNets[i].connections.size(); j++)
        {
            mNets[i].connections[j]->nodeNet = &mNets[i];
        }
    }
}

void NetList::initializeNetColors()
{
    unsigned int i, rgb[3];
    sf::Color startingColor;

    // Nets start with a blue color
    rgb[0] = 0;
    rgb[1] = 0;
    rgb[2] = 255;

    startingColor = sf::Color(rgb[0], rgb[1], rgb[2], 255);

    // When this function has been called, nets should have been randomly placed on the grid
    // Use their current position to give them their starting
    for (i = 0; i < mNets.size(); i++)
    {
        // Give the net the starting color
        mNets[i].color = startingColor;
    }
}

void NetList::initializeCellProperties()
{
    float rowToColRatio, graphicportRatio, cellSize, cellOffset, cellOppositeOffset;

    // Determine the current row to column ratio
    rowToColRatio = static_cast<float>(mNumRows) / static_cast<float>(mNumCols);
    graphicportRatio = WIN_GRAPHICPORT_HEIGHT / WIN_GRAPHICPORT_WIDTH;

    // Determine which dimension gets maximized
    if (rowToColRatio > graphicportRatio)
    {
        mCellProperties.maximizedDim = DIM_VERTICAL;
    }
    else
    {
        mCellProperties.maximizedDim = DIM_HORIZONTAL;
    }

    // Check which orientation gets maximized
    if (mCellProperties.maximizedDim == DIM_VERTICAL)
    {
        // Use rows to fill vertically
        cellSize = WIN_GRAPHICPORT_HEIGHT / static_cast<float>(mNumRows);
        // Cell offset is always half of cell size
        cellOffset = cellSize / 2.f;
        cellOppositeOffset = cellOffset + (WIN_GRAPHICPORT_WIDTH - static_cast<float>(mNumCols) * cellSize) / 2.f;
    }
    else
    {
        // Use columns to fill horizontally
        cellSize = WIN_GRAPHICPORT_WIDTH / static_cast<float>(mNumCols);
        // Cell offset is always half of cell size
        cellOffset = cellSize / 2.f;
        cellOppositeOffset = cellOffset + (WIN_GRAPHICPORT_HEIGHT - static_cast<float>(mNumRows) * cellSize) / 2.f;
    }

    // Record the cell properties
    mCellProperties.cellSize = cellSize;
    mCellProperties.cellOffset = cellOffset;
    mCellProperties.cellOppositeOffset = cellOppositeOffset;
}

std::vector<sf::RectangleShape> NetList::generateGridGeometries()
{
    std::vector<sf::RectangleShape> grid;
    unsigned int i, j;

    // Populate the grid vector with the data obtained above
    for (i = 0; i < mNumCols; i++)
    {
        for (j = 0; j < mNumRows; j++)
        {
            grid.push_back(sf::RectangleShape());
            if (mCellProperties.maximizedDim == DIM_VERTICAL)
            {
                grid.back().setPosition(
                    static_cast<float>((i*mCellProperties.cellSize) + mCellProperties.cellOppositeOffset),
                    static_cast<float>((j*mCellProperties.cellSize) + mCellProperties.cellOffset)
                );
            }
            else
            {
                grid.back().setPosition(
                    static_cast<float>((i*mCellProperties.cellSize) + mCellProperties.cellOffset),
                    static_cast<float>((j*mCellProperties.cellSize) + mCellProperties.cellOppositeOffset)
                );
            }
            grid.back().setSize(sf::Vector2f(mCellProperties.cellSize * GRID_SHRINK_FACTOR, mCellProperties.cellSize * GRID_SHRINK_FACTOR));
            grid.back().setOrigin(sf::Vector2f(mCellProperties.cellSize * GRID_SHRINK_FACTOR * 0.5f, mCellProperties.cellSize * GRID_SHRINK_FACTOR * 0.5f));
            grid.back().setFillColor(sf::Color::White);
        }
    }

    return grid;
}

std::vector<sf::Vertex> NetList::generateNetGeometries()
{
    unsigned int i, j;
    std::vector<sf::Vertex> netLines;
    std::vector<sf::Vector2f> netSegment;
    sf::Color color;
    nodeStruct_t *cellPointer[2];

    for (i = 0; i < mNets.size(); i++)
    {
        for (j = 0; j < mNets[i].connections.size() - 1; j++)
        {
            // Clear the temporary net segment vector
            netSegment.clear();

            // Get pointers to the two cells in question
            cellPointer[0] = mNets[i].connections[j];
            cellPointer[1] = mNets[i].connections[j + 1];

            // Push back the segment
            netSegment.push_back(sf::Vector2f(cellPointer[0]->drawPos.x, cellPointer[0]->drawPos.y));
            netSegment.push_back(sf::Vector2f(cellPointer[1]->drawPos.x, cellPointer[1]->drawPos.y));

            // Check to see if the segment crosses the divider, if so color it differently
            if (doesSegmentCrossDivider(netSegment))
            {
                color = sf::Color(255, 0, 0, 255);
            }
            else
            {
                color = mNets[i].color;
            }

            netLines.push_back(sf::Vertex(netSegment[0], color));
            netLines.push_back(sf::Vertex(netSegment[1], color));
        }
    }

    return netLines;
}

std::vector<sf::RectangleShape> NetList::generatePlacedNodeGeometries()
{
    std::vector<sf::RectangleShape> placedCells;
    unsigned int i;

    for (i = 0; i < mNodes.size(); i++)
    {
        placedCells.push_back(sf::RectangleShape());

        if (mCellProperties.maximizedDim == DIM_VERTICAL)
        {
            placedCells.back().setPosition(
                static_cast<float>((mNodes[i].pos.col * mCellProperties.cellSize) + mCellProperties.cellOppositeOffset),
                static_cast<float>((mNodes[i].pos.row * mCellProperties.cellSize) + mCellProperties.cellOffset)
            );
        }
        else
        {
            placedCells.back().setPosition(
                static_cast<float>((mNodes[i].pos.col * mCellProperties.cellSize) + mCellProperties.cellOffset),
                static_cast<float>((mNodes[i].pos.row * mCellProperties.cellSize) + mCellProperties.cellOppositeOffset)
            );
        }
        placedCells.back().setSize(sf::Vector2f(mCellProperties.cellSize * CELL_SHRINK_FACTOR, mCellProperties.cellSize * CELL_SHRINK_FACTOR));
        placedCells.back().setOrigin(sf::Vector2f(mCellProperties.cellSize * CELL_SHRINK_FACTOR * 0.5f, mCellProperties.cellSize * CELL_SHRINK_FACTOR * 0.5f));

        //if (state == STATE_PARTITIONING)
        //{
        placedCells.back().setFillColor(sf::Color(180, 180, 180, 255));
        //}
        //else
        //{
        //  placedCells.back().setFillColor(sf::Color(160, 193, 165, 255));
        //}
    }

    return placedCells;
}

std::vector<sf::Vertex> NetList::generatePartitionerDivider()
{
    std::vector<sf::Vertex> partitionerDivider;
    std::vector<sf::Vector2f> dividerVector;
    sf::Color color(255, 0, 0, 255); // red

    dividerVector = getDividerVector();

    partitionerDivider.push_back(sf::Vertex(dividerVector[0], color));
    partitionerDivider.push_back(sf::Vertex(dividerVector[1], color));

    return partitionerDivider;
}

void NetList::randomizeNodePlacement()
{
    unsigned int i, col, row;

    for (i = 0; i < mNodes.size(); i++)
    {
        // Put it somewhere randomly on the grid (make sure it's empty)
        do
        {
            col = static_cast<unsigned int>(getRandomInt(mNumCols));
            row = static_cast<unsigned int>(getRandomInt(mNumRows));
        }
        while (mGrid[col][row] != nullptr);

        // Update the cell's position
        updateNodePosition(i, col, row);
    }
}

void NetList::swapNodePartition(unsigned int id)
{
    unsigned int col, row;

     //Check the orientation of the divider
    if (mCellProperties.maximizedDim == DIM_VERTICAL)
    {
        // Our offset is a column
        do
        {
            if (mNodes[id].pos.col > (mNumCols / 2))
            {
                col = static_cast<unsigned int>(getRandomInt(mNumCols / 2));

            }
            else
            {
                col = static_cast<unsigned int>((mNumCols / 2) + getRandomInt(mNumCols / 2));
            }

            row = static_cast<unsigned int>(getRandomInt(mNumRows));
        }
        while (mGrid[col][row] != nullptr);
    }
    else
    {
        // Our offset is a row
        do
        {
            col = static_cast<unsigned int>(getRandomInt(mNumCols));
            if (mNodes[id].pos.row > (mNumRows / 2))
            {
                row = static_cast<unsigned int>(getRandomInt(mNumRows / 2));

            }
            else
            {
                row = static_cast<unsigned int>((mNumRows / 2) + getRandomInt(mNumRows / 2));
            }
        }
        while (mGrid[col][row] != nullptr);
    }

    // Update the cell's position
    updateNodePosition(id, col, row);
}

void NetList::getNodePosition(unsigned int id, unsigned int * col, unsigned int * row)
{
    *col = mNodes[id].pos.col;
    *row = mNodes[id].pos.row;
}

void NetList::updateNodePosition(unsigned int id, unsigned int col, unsigned int row)
{
    // Remove the cell from its old position
    mGrid[mNodes[id].pos.col][mNodes[id].pos.row] = nullptr;

    // Update the cell's grid position
    mNodes[id].pos.col = col;
    mNodes[id].pos.row = row;

    // Update the cell's drawing position
    mNodes[id].drawPos = getGridCellCoordinate(col, row);

    // Add the cell address to the grid
    mGrid[col][row] = &mNodes[id];
}

bool NetList::doesSegmentCrossDivider(std::vector<sf::Vector2f> segment)
{
    float dividerPosition;
    float segmentPos[2];
    bool returnValue;

    // Determine if the segment crosses the divider
    if (mCellProperties.maximizedDim == DIM_VERTICAL)
    {
        // Divider is vertical
        dividerPosition = WIN_GRAPHICPORT_WIDTH / 2.f;
        segmentPos[0] = segment[0].x;
        segmentPos[1] = segment[1].x;
    }
    else
    {
        // Divider is horizontal
        dividerPosition = WIN_GRAPHICPORT_HEIGHT / 2.f;
        segmentPos[0] = segment[0].y;
        segmentPos[1] = segment[1].y;
    }

    if ((segmentPos[0] < dividerPosition && segmentPos[1] > dividerPosition) || (segmentPos[1] < dividerPosition && segmentPos[0] > dividerPosition))
    {
        // Net crosses the divider
        returnValue = true;
    }
    else
    {
        returnValue = false;
    }

    return returnValue;
}

std::vector<sf::Vector2f> NetList::getDividerVector()
{
    std::vector<sf::Vector2f> dividerVector;
    drawPosStruct_t drawPos[2];

    // We will use the maximized edge for drawing the line
    if (mCellProperties.maximizedDim == DIM_VERTICAL)
    {
        drawPos[0].x = WIN_GRAPHICPORT_WIDTH / 2.f;
        drawPos[0].y = 0.f;
        drawPos[1].x = WIN_GRAPHICPORT_WIDTH / 2.f;
        drawPos[1].y = WIN_GRAPHICPORT_HEIGHT;
    }
    else
    {
        drawPos[0].x = 0.f;
        drawPos[0].y = WIN_GRAPHICPORT_HEIGHT / 2.f;
        drawPos[1].x = WIN_GRAPHICPORT_WIDTH;
        drawPos[1].y = WIN_GRAPHICPORT_HEIGHT / 2.f;
    }

    dividerVector.push_back(sf::Vector2f(drawPos[0].x, drawPos[0].y));
    dividerVector.push_back(sf::Vector2f(drawPos[1].x, drawPos[1].y));

    return dividerVector;
}

nodeStruct_t * NetList::getNodePointer(unsigned int id)
{
    return &mNodes[id];
}

void NetList::updateNetColor(unsigned int id)
{

}

drawPosStruct_t NetList::getGridCellCoordinate(unsigned int col, unsigned int row)
{
    drawPosStruct_t drawPos;

    // Depending on our current maximized direction, return the drawing position for the selected column and row
    if (mCellProperties.maximizedDim == DIM_VERTICAL)
    {
        drawPos.x = col * mCellProperties.cellSize + mCellProperties.cellOppositeOffset;
        drawPos.y = row * mCellProperties.cellSize + mCellProperties.cellOffset;
    }
    else
    {
        drawPos.x = col * mCellProperties.cellSize + mCellProperties.cellOffset;
        drawPos.y = row * mCellProperties.cellSize + mCellProperties.cellOppositeOffset;
    }

    return drawPos;
}