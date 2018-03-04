// C++ Includes
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <iterator>

// Boost includes
#include <boost/program_options.hpp>
namespace po = boost::program_options;

// SFML Includes
#include <SFML/Graphics.hpp>

// Program Includes
#include "Types.h"
#include "Partitioner.h"
#include "NetList.h"
#include "Util.h"
#include "Types.h"
#include "Constants.h"

int main(int argc, char *argv[])
{
    unsigned int i, roundNum, totalRounds, bestCut;
    parsedInputStruct_t input;
    clock_t lastTime;
    unsigned int drawModeIn, programModeIn;
    std::string filenameIn;
    drawUpdateMode_e drawMode;
    programMode_e programMode;

    // Constants
    const unsigned int NUMBER_OF_ROUNDS = 1;
    const clock_t PARTITIONER_DELAY_PERIOD_MS = 1000;
    const double GRAPHICS_DRAW_NODE_PERCENTAGE = 0.2;

    // Viewport size
    const sf::Vector2u viewportSize(
        static_cast<unsigned int>(WIN_VIEWPORT_WIDTH),
        static_cast<unsigned int>(WIN_VIEWPORT_HEIGHT));

    // Drawing components
    std::vector<sf::RectangleShape> backgroundGrid;
    std::vector<sf::RectangleShape> placedCells;
    std::vector<sf::Text> placedCellsText;
    std::vector<sf::Vertex> netLines;
    std::vector<sf::Vertex> dividerLine;

    // NetList object
    NetList *netList;
    // Partitioner object
    Partitioner *partitioner;

    // Background
    sf::RectangleShape background(sf::Vector2f(WIN_GRAPHICPORT_WIDTH, WIN_GRAPHICPORT_HEIGHT));
    background.setPosition(sf::Vector2f(0.f, 0.f));
    background.setFillColor(sf::Color(200, 200, 200, 255));

    // Log
    sf::Font font;
    sf::Text text;
    //font.loadFromFile("consola.ttf");
    font.loadFromFile("C:\\Windows\\Fonts\\consola.ttf");
    text.setFont(font);
    text.setCharacterSize(17);
    text.setFillColor(sf::Color::Green);
    text.setStyle(sf::Text::Regular);
    text.setPosition(sf::Vector2f(0.f + WIN_INFOPORT_PADDING, WIN_VIEWPORT_HEIGHT - WIN_INFOPORT_HEIGHT + WIN_INFOPORT_PADDING));

    // Separator
    sf::Vertex line[] =
    {
        sf::Vertex(sf::Vector2f(0.f, WIN_VIEWPORT_HEIGHT - WIN_INFOPORT_HEIGHT)),
        sf::Vertex(sf::Vector2f(WIN_VIEWPORT_WIDTH, WIN_VIEWPORT_HEIGHT - WIN_INFOPORT_HEIGHT))
    };

    // Instantiate the Partitioner
    partitioner = new Partitioner();

    // Declare the supported options
    po::options_description desc("Usage");
    desc.add_options()
        ("help", "produce help message")
        ("input-file", po::value<std::string>(&filenameIn)->default_value("..\\benchmarks\\C880.txt"),
            "the input file")
        ("program-mode,p", po::value<unsigned int>(&programModeIn)->default_value(static_cast<unsigned int>(PROGRAM_MODE_GUI)),
            "set the program mode")
        ("draw-mode,d", po::value<unsigned int>(&drawModeIn)->default_value(static_cast<unsigned int>(DRAW_EVERY_NODE_PERCENT)),
            "set the draw mode")
        ("rounds,r", po::value<unsigned int>(&totalRounds)->default_value(NUMBER_OF_ROUNDS),
            "set the number of rounds");
    // This makes the input file a positional argument
    po::positional_options_description p;
    p.add("input-file", -1);
    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).
        options(desc).positional(p).run(), vm);
    po::notify(vm);
    
    if (vm.count("help"))
    {
        std::cout << desc << std::endl;
        return 1;
    }
    if (vm.count("input-file"))
    {
        std::cout << "Input file: "
            << vm["input-file"].as<std::string>() << std::endl;
    }
    partitioner->mFilename = filenameIn;
    if (vm.count("program-mode"))
    {
        if (programModeIn >= PROGRAM_MODE_NUM)
        {
            std::cout << "Invalid program mode: " << programModeIn << std::endl;
            return -1;
        }
        else
        {
            std::cout << "Program mode: " << programModeIn << std::endl;
        }
    }
    programMode = static_cast<programMode_e>(programModeIn);
    if (vm.count("draw-mode"))
    {
        if (programModeIn >= DRAW_MODE_NUM)
        {
            std::cout << "Invalid draw mode: " << programModeIn << std::endl;
            return -1;
        }
        else
        {
            std::cout << "Draw mode: " << programModeIn << std::endl;
        }
    }
    drawMode = static_cast<drawUpdateMode_e>(drawModeIn);

    std::cout << "Will run for " << totalRounds << " rounds." << std::endl;

    // Parse the input file
    if (!partitioner->parseInputFile())
    {
        return -1;
    }
    input = partitioner->getParsedInput();
    // Instantiate the NetList
    netList = new NetList(input);

    lastTime = clock();
    roundNum = 0;
    bestCut = 0xDEADBEEF;

    if (programMode == PROGRAM_MODE_GUI)
    {
        // Get a grid, only need to do this once since it is static
        backgroundGrid = netList->generateGridGeometries();
        // Create our render window object
        // Give it a default type (titlebar, close button)
        sf::RenderWindow window(sf::VideoMode(
            static_cast<unsigned int>(WIN_VIEWPORT_WIDTH),
            static_cast<unsigned int>(WIN_VIEWPORT_HEIGHT)),
            "Partitioner", sf::Style::Titlebar | sf::Style::Close | sf::Style::Resize);
        window.setView(calcView(window.getSize(), viewportSize));

        // Do partitioning and output results
        while (window.isOpen())
        {
            sf::Event event;
            while (window.pollEvent(event))
            {
                if (event.type == sf::Event::Closed)
                    window.close();
                if (event.type == sf::Event::Resized)
                    window.setView(calcView(sf::Vector2u(event.size.width, event.size.height), viewportSize));
            }

            // Get divider line
            dividerLine = netList->generatePartitionerDivider();
            // Get net lines
            netLines = netList->generateNetGeometries();
            // Get placed cells
            placedCells = netList->generatePlacedNodeGeometries();
            // Get placed cells text
            placedCellsText = netList->generatePlacedNodeText();

            // Clear window
            window.clear();
            // Draw background
            window.draw(background);
            // Draw infoport
            text.setString(partitioner->getInfoportString());
            window.draw(text);
            // Draw separator
            window.draw(line, 2, sf::Lines);
            // Draw grid
            for (i = 0; i < backgroundGrid.size(); i++)
            {
                window.draw(backgroundGrid[i]);
            }

            // Draw net lines
            window.draw(&netLines.front(), netLines.size(), sf::Lines);

            // Draw placed cells and their text
            for (i = 0; i < placedCells.size(); i++)
            {
                window.draw(placedCells[i]);
                window.draw(placedCellsText[i]);
            }

            // Draw divider
            window.draw(&dividerLine.front(), dividerLine.size(), sf::Lines);

            // Display window
            window.display();

            // Run the partitioning for a percentage of the nodes that we have (tradeoff between screen update and partitioner working)
            switch (drawMode)
            {
                // Partitioner runs every graphic update
                case DRAW_EVERY_TICK:
                    partitioner->doPartitioning(*netList);
                    break;
                    // Partitioner runs for a percentage of the total nodes
                case DRAW_EVERY_NODE_PERCENT:
                    for (i = 0; i < netList->getNumNodes() * GRAPHICS_DRAW_NODE_PERCENTAGE; i++)
                    {
                        partitioner->doPartitioning(*netList);
                    }
                    break;
                    // Partioner for a period of some seconds
                case DRAW_EVERY_PERIOD:
                    if (clock() - lastTime > PARTITIONER_DELAY_PERIOD_MS)
                    {
                        partitioner->doPartitioning(*netList);
                        lastTime = clock();
                    }
                default:
                    // Should never be here, do nothing
                    break;
            }

            // Check if partitioner is done (only do this for totalRounds > 1)
            if (totalRounds > 1 && partitioner->getState() == STATE_FINISHED)
            {
                // We're done a round
                roundNum++;
                // See if we did better than previously
                if (bestCut > partitioner->mCurrentCutSize)
                {
                    bestCut = partitioner->mCurrentCutSize;
                }
                // Output result
                std::cout << "Round " << roundNum << " finished. Got " << partitioner->mCurrentCutSize << " cuts in " << partitioner->mEndTime - partitioner->mStartTime << " ms" << std::endl;
                // Re-start
                partitioner->setState(STATE_INIT);
                if (roundNum >= totalRounds)
                {
                    // We're done for good.
                    std::cout << "Done for good! After " << roundNum << " rounds, best cut was: " << bestCut << std::endl;
                    return 0;
                }
            }
        }
    }
    else if (programMode == PROGRAM_MODE_CLI)
    {
        do
        {
            // Do partioner
            partitioner->doPartitioning(*netList);
            // Check if partitioner is done (only do this for totalRounds > 1)
            if (totalRounds > 1 && partitioner->getState() == STATE_FINISHED)
            {
                // We're done a round
                roundNum++;
                // See if we did better than previously
                if (bestCut > partitioner->mCurrentCutSize)
                {
                    bestCut = partitioner->mCurrentCutSize;
                }
                // Output result
                std::cout << "Round " << roundNum << " finished. Got " << partitioner->mCurrentCutSize << " cuts in " << partitioner->mEndTime - partitioner->mStartTime << " ms" << std::endl;
                // Re-start
                partitioner->setState(STATE_INIT);
            }
        }
        while (roundNum < totalRounds);

        // We're done for good.
        std::cout << "Done for good! After " << roundNum << " rounds, best cut was: " << bestCut << std::endl;
        return 0;
    }
    else if (programMode == PROGRAM_MODE_TEST)
    {
        std::cout << "RUNNING BUILT IN SELF TEST!" << std::endl;
        std::cout << "Using benchmark C880 for testing..." << std::endl;
        // Check to make sure the parser made sense of C880
        if (input.numCols != 20)
        {
            std::cout << "FAIL! Parsed incorrect number of columns: " << input.numCols << std::endl;
            return -1;
        }
        if (input.numRows != 15)
        {
            std::cout << "FAIL! Parsed incorrect number of row: " << input.numRows << std::endl;
            return -1;
        }
        if (input.numNodes != 260)
        {
            std::cout << "FAIL! Parsed incorrect number of nodes: " << input.numNodes << std::endl;
            return -1;
        }
        if (input.numConnections != 234)
        {
            std::cout << "FAIL! Parsed incorrect number of nets: " << input.numConnections << std::endl;
            return -1;
        }
        // Check that the netlist made the correct vectors
        if (netList->getNumCols() != 40)
        {
            std::cout << "FAIL! Netlist incorrect number of columns: " << netList->getNumCols() << std::endl;
            return -1;
        }
        if (netList->getNumRows() != 30)
        {
            std::cout << "FAIL! Netlist incorrect number of row: " << netList->getNumRows() << std::endl;
            return -1;
        }
        if (netList->getNumNodes() != 260)
        {
            std::cout << "FAIL! Netlist incorrect number of nodes: " << netList->getNumNodes() << std::endl;
            return -1;
        }
        if (netList->getNumNets() != 234)
        {
            std::cout << "FAIL! Netlist incorrect number of nets: " << netList->getNumNets() << std::endl;
            return -1;
        }
        std::cout << "All test passed!" << std::endl;
        return 0;
    }
    else
    {
        std::cout << "Invalid program mode: " << programMode << std::endl;
        return -1;
    }

    return 0;
}