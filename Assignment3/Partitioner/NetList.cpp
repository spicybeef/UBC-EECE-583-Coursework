#include "NetList.h"

NetList::NetList(parsedInputStruct_t parsedInput)
{
	// Copy into the object the file's parsed input
	mParsedInput = parsedInput;

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
	


	mGrid.clear();

	// Generate the grid model
	for (i = 0; i < mNumCols; i++)
	{
		mGrid.push_back(gridColVec());
		for (j = 0; j < mNumRows; j++)
		{
			mGrid[i].push_back(nullptr);
		}
	}
}

void NetList::initializeNodes()
{
	unsigned int i;

	mNodes.clear();

	// For each cell
	for (i = 0; i < mNodes.size(); i++)
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

	// Nets start with a red color
	rgb[0] = 255;
	rgb[1] = 0;
	rgb[2] = 0;

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
	rowToColRatio = static_cast<float>(mCellProperties.numRows) / static_cast<float>(mCellProperties.numCols);
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
		cellSize = WIN_GRAPHICPORT_HEIGHT / static_cast<float>(mCellProperties.numRows);
		// Cell offset is always half of cell size
		cellOffset = cellSize / 2.f;
		cellOppositeOffset = cellOffset + (WIN_GRAPHICPORT_WIDTH - static_cast<float>(mCellProperties.numCols) * cellSize) / 2.f;
	}
	else
	{
		// Use columns to fill horizontally
		cellSize = WIN_GRAPHICPORT_WIDTH / static_cast<float>(mCellProperties.numCols);
		// Cell offset is always half of cell size
		cellOffset = cellSize / 2.f;
		cellOppositeOffset = cellOffset + (WIN_GRAPHICPORT_HEIGHT - static_cast<float>(mCellProperties.numRows) * cellSize) / 2.f;
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
	sf::Color color;
	nodeStruct_t *cellPointer[2];

	for (i = 0; i < mNets.size(); i++)
	{
		color = mNets[i].color;
		for (j = 0; j < mNets[i].connections.size() - 1; j++)
		{
			cellPointer[0] = mNets[i].connections[j];
			cellPointer[1] = mNets[i].connections[j + 1];
			netLines.push_back(sf::Vertex(sf::Vector2f(cellPointer[0]->drawPos.x, cellPointer[0]->drawPos.y), color));
			netLines.push_back(sf::Vertex(sf::Vector2f(cellPointer[1]->drawPos.x, cellPointer[1]->drawPos.y), color));
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
		// Only look at cells with nets
		if (mNodes[i].nodeNet == NULL)
		{
			continue;
		}

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
		//	placedCells.back().setFillColor(sf::Color(180, 180, 180, 255));
		//}
		//else
		//{
		//	placedCells.back().setFillColor(sf::Color(160, 193, 165, 255));
		//}
	}

	return placedCells;
}

void NetList::randomizeNodePlacement()
{
}

void NetList::swapNodePartition(unsigned int id)
{
}

void NetList::getNodePosition(unsigned int id, unsigned int * col, unsigned int * row)
{
}

void NetList::updateNodePosition(unsigned int id, unsigned int col, unsigned int row)
{
}

void NetList::updateNetColor(unsigned int id)
{
}

drawPosStruct_t NetList::getGridCellCoordinate(cellPropertiesStruct_t cellProperties, unsigned int col, unsigned int row)
{
	return drawPosStruct_t();
}
