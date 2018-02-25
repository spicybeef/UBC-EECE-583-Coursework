#include <algorithm>

#include "NetList.h"
#include "Util.h"
#include "Types.h"
#include "Constants.h"

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
    initializeNodeNeighbours();
    initializeNetColors();
    initializeCellProperties();

    // Create the font
    //mFont.loadFromFile("consola.ttf");
    mFont.loadFromFile("C:\\Windows\\Fonts\\consola.ttf");
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
        // Initialize node gain to 0
        mNodes.back().gain = 0;
        // Set the node unlocked
        unlockNode(i);
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
    unsigned int i, j, k;
    bool found;

    // Go through the nets and its connections, and update all of the nodes' neighbors
    for (i = 0; i < mNets.size(); i++)
    {
        for (j = 0; j < mNets[i].connections.size() - 1; j++)
        {
            // Make these nodes neighbors
            // Make sure the connection doesn't already exist
            // This feels inefficient, likely a better way to check
            // Will let it slide since this only happens on NetList construction

            // First pass for the forward connection...
            found = false;
            for (k = 0; k < mNets[i].connections[j]->neighbors.size(); k++)
            {
                if (mNets[i].connections[j]->neighbors[k] == mNets[i].connections[j + 1])
                {
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                mNets[i].connections[j]->neighbors.push_back(mNets[i].connections[j + 1]);
            }
            else
            {
                found = false;
            }

            // Second pass for the backwards connection...
            found = false;
            for (k = 0; k < mNets[i].connections[j + 1]->neighbors.size(); k++)
            {
                if (mNets[i].connections[j + 1]->neighbors[k] == mNets[i].connections[j])
                {
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                mNets[i].connections[j + 1]->neighbors.push_back(mNets[i].connections[j]);
            }
            else
            {
                found = false;
            }
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

            // Check to see if the segment crosses the divider; if so, color it differently
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

        if (mNodes[i].state == NODE_STATE_LOCKED)
        {
            placedCells.back().setFillColor(sf::Color(244, 86, 66, 255)); // Redish
        }
        else
        {
            placedCells.back().setFillColor(sf::Color(160, 193, 165, 255)); // Greenish
        }
    }

    return placedCells;
}

std::vector<sf::Text> NetList::generatePlacedNodeText()
{
    std::vector<sf::Text> placedCellsText;
    unsigned int i;

    for (i = 0; i < mNodes.size(); i++)
    {
        placedCellsText.push_back(sf::Text());

        placedCellsText.back().setString(std::to_string(mNodes[i].gain));

        if (mCellProperties.maximizedDim == DIM_VERTICAL)
        {
            placedCellsText.back().setPosition(
                static_cast<float>((mNodes[i].pos.col * mCellProperties.cellSize) + mCellProperties.cellOppositeOffset),
                static_cast<float>((mNodes[i].pos.row * mCellProperties.cellSize) + mCellProperties.cellOffset)
            );
        }
        else
        {
            placedCellsText.back().setPosition(
                static_cast<float>((mNodes[i].pos.col * mCellProperties.cellSize) + mCellProperties.cellOffset),
                static_cast<float>((mNodes[i].pos.row * mCellProperties.cellSize) + mCellProperties.cellOppositeOffset)
            );
        }
        placedCellsText.back().setFont(mFont);
        placedCellsText.back().setCharacterSize(15);
        placedCellsText.back().setFillColor(sf::Color::Black);
        placedCellsText.back().setStyle(sf::Text::Regular);

        placedCellsText.back().setOrigin(sf::Vector2f(mCellProperties.cellSize * CELL_SHRINK_FACTOR * 0.5f, mCellProperties.cellSize * CELL_SHRINK_FACTOR * 0.5f));
    }

    return placedCellsText;
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
    posStruct_t pos;
    unsigned int i, count;

    // Shuffle the node list
    std::random_shuffle(mNodes.begin(), mNodes.end());

    count = 0;
    for (i = 0; i < mNodes.size(); i++)
    {
        // Put it somewhere randomly on the grid (make sure it's empty)
        // We must distribute them equally across the partition
        // We will use the first bit of a counter to toggle between ranges
        if (mCellProperties.maximizedDim == DIM_VERTICAL)
        {
            do
            {
                // Column will switch between two ranges
                pos.col = static_cast<unsigned int>(getRandomInt(mNumCols / 2) + (count & 0x1) * (mNumCols / 2));
                // Row is always full range random
                pos.row = static_cast<unsigned int>(getRandomInt(mNumRows));
            }
            while (mGrid[pos.col][pos.row] != nullptr);
        }
        else
        {
            do
            {
                // Col is always full range random
                pos.col = static_cast<unsigned int>(getRandomInt(mNumCols));
                // Row will switch between two ranges
                pos.row = static_cast<unsigned int>(getRandomInt(mNumRows / 2) + (count & 0x1) * (mNumRows / 2));
            }
            while (mGrid[pos.col][pos.row] != nullptr);
        }
        // Increment the count
        count++;

        // Update the cell's position
        updateNodePosition(i, pos);
    }

    // Update all the gains
    updateAllNodeGains();
}

void NetList::swapNodePartition(unsigned int id)
{
    posStruct_t pos;
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
    pos.col = col;
    pos.row = row;
    updateNodePosition(id, pos);
}

unsigned int NetList::getNodePartition(unsigned int id)
{
    unsigned int partition;
    // Partition 0 will be the lower ranger, and partition 1 will be the upper range
    // The current maximized dimension determines the partition we are in
    if (mCellProperties.maximizedDim == DIM_VERTICAL)
    {
        // Column will determine the partition number
        if (mNodes[id].pos.col < mNumCols / 2)
        {
            partition = 0;
        }
        else
        {
            partition = 1;
        }
    }
    else
    {
        // Row will determine the partition number
        if (mNodes[id].pos.row < mNumRows / 2)
        {
            partition = 0;
        }
        else
        {
            partition = 1;
        }
    }

    return partition;
}

posStruct_t NetList::getNodePosition(unsigned int id)
{
    posStruct_t pos;

    pos.col = mNodes[id].pos.col;
    pos.row = mNodes[id].pos.row;

    return pos;
}

void NetList::updateNodePosition(unsigned int id, posStruct_t pos)
{
    // Remove the cell from its old position
    mGrid[mNodes[id].pos.col][mNodes[id].pos.row] = nullptr;

    // Update the cell's grid position
    mNodes[id].pos.col = pos.col;
    mNodes[id].pos.row = pos.row;

    // Update the cell's drawing position
    mNodes[id].drawPos = getGridCellCoordinate(pos.col, pos.row);

    // Add the cell address to the grid
    mGrid[pos.col][pos.row] = &mNodes[id];
}

void NetList::lockNode(unsigned int id)
{
    mNodes[id].state = NODE_STATE_LOCKED;
}

void NetList::unlockNode(unsigned int id)
{
    mNodes[id].state = NODE_STATE_UNLOCKED;
}

void NetList::unlockAllNodes()
{
    unsigned int i;

    for (i = 0; i < mNodes.size(); i++)
    {
        mNodes[i].state = NODE_STATE_UNLOCKED;
    }
}

bool NetList::isNodeLocked(unsigned int id)
{
    if (mNodes[id].state == NODE_STATE_LOCKED)
    {
        return true;
    }
    else
    {
        return false;
    }
}

unsigned int NetList::calculateCurrentCutSize()
{
    unsigned int i, j, cutSize;
    std::vector<sf::Vector2f> netSegment;
    nodeStruct_t *cellPointer[2];

    cutSize = 0;
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

            // Check to see if the segment crosses the divider; if so, count it
            if (doesSegmentCrossDivider(netSegment))
            {
                cutSize++;
            }
        }
    }

    return cutSize;
}

int NetList::calculateTotalGain()
{
    unsigned int i;
    int totalGain;

    updateAllNodeGains();

    totalGain = 0;
    for (i = 0; i < mNodes.size(); i++)
    {
        totalGain += mNodes[i].gain;
    }

    return totalGain;
}

int NetList::calculateNodeGain(unsigned int id)
{
    std::vector<sf::Vector2f> netSegment;
    nodeStruct_t * neighborPointer;
    unsigned int i;
    int currentGain = 0;

    // 
    // Origin of the segment is the node we are on
    netSegment.push_back(sf::Vector2f(mNodes[id].drawPos.x, mNodes[id].drawPos.y));
    // Push back the other end with a dummy
    netSegment.push_back(sf::Vector2f());
    // Go through each neighbour and check if we cross the boundary
    for (i = 0; i < mNodes[id].neighbors.size(); i++)
    {
        neighborPointer = mNodes[id].neighbors[i];
        netSegment[1] = sf::Vector2f(neighborPointer->drawPos.x, neighborPointer->drawPos.y);

        if (doesSegmentCrossDivider(netSegment))
        {
            currentGain++;
        }
        else
        {
            currentGain--;
        }
    }

    mNodes[id].gain = currentGain;

    return currentGain;
}

int NetList::getNodeGain(unsigned int id)
{
    // This won't recalculate
    return mNodes[id].gain;
}

void NetList::updateAllNodeGains()
{
    unsigned int i;

    for (i = 0; i < mNodes.size(); i++)
    {
        calculateNodeGain(i);
    }
}

unsigned int NetList::getNumNodes()
{
    return static_cast<unsigned int>(mNodes.size());
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