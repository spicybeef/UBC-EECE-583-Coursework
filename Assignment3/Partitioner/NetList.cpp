#include <algorithm>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <numeric>

#include "NetList.h"
#include "Util.h"
#include "Types.h"
#include "Constants.h"

#define GAIN_MOD 1

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
    initializeNetSegments();
    initializeNodeNeighbours();
    initializeNetColors();
    initializeCellProperties();

    // Create the font
    //mFont.loadFromFile("consola.ttf");
    mFont.loadFromFile("C:\\Windows\\Fonts\\consola.ttf");

    // Output some information
    std::cout << "Construction of NetList complete! Summary:" << std::endl;
    std::cout << "Nodes: " << mNodes.size() << std::endl;
    std::cout << "Nets: " << mNets.size() << std::endl;
    std::cout << "Net Segments: " << mNetSegments.size() << std::endl;
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

    // For each node
    for (i = 0; i < mParsedInput.numNodes; i++)
    {
        // Add an empty node
        mNodes.push_back(nodeStruct_t());
        // Initialize node gain to 0
        mNodes.back().gain = 0;
        // Set the node unlocked
        unlockNode(i);
    }
}

void NetList::initializeNets()
{
    unsigned int i, j;

    // Clear any existing nets
    mNets.clear();

    // Go through the parsed input, and add each net
    for (i = 0; i < mParsedInput.nets.size(); i++)
    {
        // Push a fresh net to the placer
        mNets.push_back(netStruct_t());

        for (j = 0; j < mParsedInput.nets[i].size(); j++)
        {
            // Give the net its node index
            mNets.back().nodes.push_back(mParsedInput.nets[i][j]);
        }
    }
}

void NetList::initializeNetSegments()
{
    unsigned int i, j, totalSegments;

    // Clear net segments vector
    mNetSegments.clear();

    totalSegments = 0;
    // Go through the nets and its connections, and create a node segment for each
    for (i = 0; i < mNets.size(); i++)
    {
        // Push back net segments
        for (j = 0; j < mNets[i].nodes.size() - 1; j++)
        {
            // Push back the segment onto the segment vector
            mNetSegments.push_back(netSegmentStruct_t());

            // Push back pointers to the nodes on either side of the segment
            mNetSegments.back().nodes.push_back(mNets[i].nodes[j]);
            mNetSegments.back().nodes.push_back(mNets[i].nodes[j + 1]);

            // Give the net segment its net id
            mNetSegments.back().net = i;

            // Push back index onto the net's segment vector
            mNets[i].segments.push_back(totalSegments);

            // Push back index onto each of the nodes' segment vectors
            getNodePointer(mNets[i].nodes[j])->segments.push_back(totalSegments);
            getNodePointer(mNets[i].nodes[j + 1])->segments.push_back(totalSegments);

            totalSegments++;
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
        for (j = 0; j < mNets[i].nodes.size() - 1; j++)
        {
            // Make these nodes neighbors
            // Make sure the connection doesn't already exist
            // This feels inefficient, likely a better way to check
            // Will let it slide since this only happens on NetList construction

            // First pass for the forward connection...
            found = false;
            for (k = 0; k < getNodePointer(mNets[i].nodes[j])->neighbors.size(); k++)
            {
                if (getNodePointer(mNets[i].nodes[j])->neighbors[k] == mNets[i].nodes[j + 1])
                {
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                getNodePointer(mNets[i].nodes[j])->neighbors.push_back(mNets[i].nodes[j + 1]);
            }
            else
            {
                found = false;
            }

            // Second pass for the backwards connection...
            found = false;
            for (k = 0; k < getNodePointer(mNets[i].nodes[j + 1])->neighbors.size(); k++)
            {
                if (getNodePointer(mNets[i].nodes[j + 1])->neighbors[k] == mNets[i].nodes[j])
                {
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                getNodePointer(mNets[i].nodes[j + 1])->neighbors.push_back(mNets[i].nodes[j]);
            }
            else
            {
                found = false;
            }
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

    // When this function has been called, nets should have been randomly placed on the grid
    // Use their current position to give them their starting
    for (i = 0; i < mNets.size(); i++)
    {
        //rgb[0] = getRandomInt(256);
        //rgb[1] = getRandomInt(256);
        //rgb[2] = getRandomInt(256);
        startingColor = sf::Color(rgb[0], rgb[1], rgb[2], 255);
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
    nodeStruct_t *nodePointer[2];

    for (i = 0; i < mNets.size(); i++)
    {
        for (j = 0; j < mNets[i].segments.size(); j++)
        {
            // Clear the temporary net segment vector
            netSegment.clear();

            // Get pointers to the two nodes in question
            nodePointer[0] = getNodePointer(getNetSegmentPointer(mNets[i].segments[j])->nodes[0]);
            nodePointer[1] = getNodePointer(getNetSegmentPointer(mNets[i].segments[j])->nodes[1]);

            // Push back the segment
            netSegment.push_back(sf::Vector2f(nodePointer[0]->drawPos.x, nodePointer[0]->drawPos.y));
            netSegment.push_back(sf::Vector2f(nodePointer[1]->drawPos.x, nodePointer[1]->drawPos.y));

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
    std::vector<sf::RectangleShape> placedNodes;
    unsigned int i;

    for (i = 0; i < mNodes.size(); i++)
    {
        placedNodes.push_back(sf::RectangleShape());

        if (mCellProperties.maximizedDim == DIM_VERTICAL)
        {
            placedNodes.back().setPosition(
                static_cast<float>((mNodes[i].pos.col * mCellProperties.cellSize) + mCellProperties.cellOppositeOffset),
                static_cast<float>((mNodes[i].pos.row * mCellProperties.cellSize) + mCellProperties.cellOffset)
            );
        }
        else
        {
            placedNodes.back().setPosition(
                static_cast<float>((mNodes[i].pos.col * mCellProperties.cellSize) + mCellProperties.cellOffset),
                static_cast<float>((mNodes[i].pos.row * mCellProperties.cellSize) + mCellProperties.cellOppositeOffset)
            );
        }
        placedNodes.back().setSize(sf::Vector2f(mCellProperties.cellSize * CELL_SHRINK_FACTOR, mCellProperties.cellSize * CELL_SHRINK_FACTOR));
        placedNodes.back().setOrigin(sf::Vector2f(mCellProperties.cellSize * CELL_SHRINK_FACTOR * 0.5f, mCellProperties.cellSize * CELL_SHRINK_FACTOR * 0.5f));

        if (mNodes[i].state == NODE_STATE_LOCKED)
        {
            placedNodes.back().setFillColor(sf::Color(244, 86, 66, 255)); // Redish
        }
        else
        {
            placedNodes.back().setFillColor(sf::Color(160, 193, 165, 255)); // Greenish
        }
    }

    return placedNodes;
}

std::vector<sf::Text> NetList::generatePlacedNodeText()
{
    std::stringstream stringStream;
    std::vector<sf::Text> placedNodeText;
    unsigned int i;

    for (i = 0; i < mNodes.size(); i++)
    {
        stringStream.str(std::string());
        placedNodeText.push_back(sf::Text());

        stringStream << i << " [" << std::to_string(mNodes[i].gain) << "]";
        placedNodeText.back().setString(stringStream.str());

        if (mCellProperties.maximizedDim == DIM_VERTICAL)
        {
            placedNodeText.back().setPosition(
                static_cast<float>((mNodes[i].pos.col * mCellProperties.cellSize) + mCellProperties.cellOppositeOffset),
                static_cast<float>((mNodes[i].pos.row * mCellProperties.cellSize) + mCellProperties.cellOffset)
            );
        }
        else
        {
            placedNodeText.back().setPosition(
                static_cast<float>((mNodes[i].pos.col * mCellProperties.cellSize) + mCellProperties.cellOffset),
                static_cast<float>((mNodes[i].pos.row * mCellProperties.cellSize) + mCellProperties.cellOppositeOffset)
            );
        }
        placedNodeText.back().setFont(mFont);
        placedNodeText.back().setCharacterSize(9);
        placedNodeText.back().setFillColor(sf::Color::Black);
        placedNodeText.back().setStyle(sf::Text::Regular);

        placedNodeText.back().setOrigin(sf::Vector2f(mCellProperties.cellSize * CELL_SHRINK_FACTOR * 0.5f, mCellProperties.cellSize * CELL_SHRINK_FACTOR * 0.5f));
    }

    return placedNodeText;
}

std::vector<sf::Vertex> NetList::generatePartitionerDivider()
{
    std::vector<sf::Vertex> partitionerDivider;
    std::vector<sf::Vector2f> dividerVector;
    sf::Color color(0, 255, 0, 255); // green

    dividerVector = getDividerVector();

    partitionerDivider.push_back(sf::Vertex(dividerVector[0], color));
    partitionerDivider.push_back(sf::Vertex(dividerVector[1], color));

    return partitionerDivider;
}

void NetList::randomizeNodePlacement()
{
    posStruct_t pos;
    unsigned int i, count;
    std::vector<unsigned int> shuffledIndices(getNumNodes());

    // Shuffle an index list
    std::iota(shuffledIndices.begin(), shuffledIndices.end(), 0);
    std::random_shuffle(shuffledIndices.begin(), shuffledIndices.end());

    count = 0;
    for (i = 0; i < shuffledIndices.size(); i++)
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

        // Update the node's position
        updateNodePosition(shuffledIndices[i], pos);
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
            if (mNodes[id].pos.col > ((mNumCols / 2) - 1))
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
            if (mNodes[id].pos.row > ((mNumRows / 2) - 1))
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

    // Update the node's position
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
    // Remove the node from its old position
    mGrid[mNodes[id].pos.col][mNodes[id].pos.row] = nullptr;

    // Update the node's grid position
    mNodes[id].pos.col = pos.col;
    mNodes[id].pos.row = pos.row;

    // Update the node's drawing position
    mNodes[id].drawPos = getGridCellCoordinate(pos.col, pos.row);

    // Add the node address to the grid
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
    nodeStruct_t *nodePointer[2];

    cutSize = 0;
    for (i = 0; i < mNets.size(); i++)
    {
        for (j = 0; j < mNets[i].segments.size(); j++)
        {
            // Clear the temporary net segment vector
            netSegment.clear();

            // Get pointers to the two nodes in question
            nodePointer[0] = getNodePointer(getNetSegmentPointer(mNets[i].segments[j])->nodes[0]);
            nodePointer[1] = getNodePointer(getNetSegmentPointer(mNets[i].segments[j])->nodes[1]);

            // Push back the segment
            netSegment.push_back(sf::Vector2f(nodePointer[0]->drawPos.x, nodePointer[0]->drawPos.y));
            netSegment.push_back(sf::Vector2f(nodePointer[1]->drawPos.x, nodePointer[1]->drawPos.y));

            // Check to see if the segment crosses the divider; if so, count it
            if (doesSegmentCrossDivider(netSegment))
            {
                cutSize++;
                // We only count one crossing, any others don't matter
                break;
            }
        }
    }

    return cutSize;
}

int NetList::calculateNodeGain(unsigned int id)
{
    std::vector<sf::Vector2f> netSegment;
    std::vector<unsigned int> crossingNets;
    unsigned int i, j, neighborNodeId, segmentNet, nodePartition, otherPartition;
    nodeStruct_t *nodePointer[2];
    int currentGain = 0;
    std::vector<unsigned int> netNodePartitions[2];

#if GAIN_MOD
    // Record the node's current partition
    nodePartition = getNodePartition(id);
    // Get the other partition
    if (nodePartition == 0)
    {
        otherPartition = 1;
    }
    else
    {
        otherPartition = 0;
    }
    netNodePartitions[nodePartition].push_back(id);
    // Determine which partition each of the node's neighbors belongs to
    // This is to try to encourage connected nets to go to the partition with the most of its friends
    for (i = 0; i < getNodePointer(id)->neighbors.size(); i++)
    {
        netNodePartitions[getNodePartition(getNodePointer(id)->neighbors[i])].push_back(getNodePointer(id)->neighbors[i]);
    }
    // Check if we have an imbalance
    if (netNodePartitions[nodePartition].size() == 0 || netNodePartitions[otherPartition].size() == 0)
    {
        // Nope, all of our nodes are on one side, this is good so discourage a swap by giving the node a very low gain
        currentGain = -999;
    }
    // There's room for improvement by swapping it (or leaving it where it is)
    else
    {
        // If our friends are mostly in the other partition, encourage a swap by giving it a large gain based on the difference
        if (netNodePartitions[nodePartition].size() < netNodePartitions[otherPartition].size())
        {
            currentGain += 2 * (static_cast<int>(netNodePartitions[otherPartition].size()) - static_cast<int>(netNodePartitions[nodePartition].size()));
        }
        // Otherwise discourage it
        else
        {
            currentGain -= 2 * (static_cast<int>(netNodePartitions[nodePartition].size()) - static_cast<int>(netNodePartitions[otherPartition].size()));
        }
    }
#endif //GAINMOD

    // The same net crossing the partition must not be counted twice
    // For the given node, go through its net segments
    for (i = 0; i < getNodePointer(id)->segments.size(); i++)
    {
        // Record the segment's net
        segmentNet = getNetSegmentPointer(getNodePointer(id)->segments[i])->net;

        // Get the neighbor's node ID through the segment (one of them should be this one)
        j = 0;
        do
        {
            neighborNodeId = getNetSegmentPointer(getNodePointer(id)->segments[i])->nodes[j++];
        }
        while (neighborNodeId != id);

        // Clear the temporary net segment vector
        netSegment.clear();

        // Get pointers to the two nodes in question
        nodePointer[0] = getNodePointer(id);
        nodePointer[1] = getNodePointer(neighborNodeId);

        // Push back the segment
        netSegment.push_back(sf::Vector2f(nodePointer[0]->drawPos.x, nodePointer[0]->drawPos.y));
        netSegment.push_back(sf::Vector2f(nodePointer[1]->drawPos.x, nodePointer[1]->drawPos.y));

        // Check to see if the segment crosses the divider
        if (doesSegmentCrossDivider(netSegment))
        {
            // Net is crossing!
            // Linearly search through crossing nets to see if we've encountered it before
            if (std::find(crossingNets.begin(), crossingNets.end(), segmentNet) == crossingNets.end())
            {
                // The net hasn't been seen crossing yet
                crossingNets.push_back(segmentNet);
                // This net increments our gain, but no further occurence of it will increase it
                currentGain++;
            }
            // Net was already seen crossing, so it doesn't count towards the gain
        }
        else
        {
            // Net does not cross partition, it reduces the gain
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

netStruct_t * NetList::getNetPointer(unsigned int id)
{
    return &mNets[id];
}

netSegmentStruct_t * NetList::getNetSegmentPointer(unsigned int id)
{
    return &mNetSegments[id];
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