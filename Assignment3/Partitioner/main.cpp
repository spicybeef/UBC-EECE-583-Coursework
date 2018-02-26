// C++ Includes
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

// SFML Includes
#include <SFML/Graphics.hpp>

// Program Includes
#include "Types.h"
#include "Partitioner.h"
#include "NetList.h"
#include "Util.h"
#include "Types.h"
#include "Constants.h"

int main(int argc, char **argv)
{
    unsigned int i;
    parsedInputStruct_t input;
    clock_t lastTime;
    //const runMode_e runMode = EVERY_TICK;
    const runMode_e runMode = EVERY_NODE_PERCENT;
    //const runMode_e runMode = EVERY_PERIOD;

    const clock_t PARTITIONER_DELAY_PERIOD_MS = 1000;

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

    // Filename to read in is the second argument
    // If no argument is given, load up a default input file
    if (!argv[1])
    {
        //partitioner->mFilename = const_cast<char *>("..\\benchmarks\\test.txt");
        //partitioner->mFilename = const_cast<char *>("..\\benchmarks\\cm138a.txt");
        partitioner->mFilename = const_cast<char *>("..\\benchmarks\\cm151a.txt");
        //partitioner->mFilename = const_cast<char *>("..\\benchmarks\\apex4.txt");
    }
    else
    {
        partitioner->mFilename = argv[1];
    }

    // Parse the input file
    if (!partitioner->parseInputFile())
    {
        return -1;
    }
    input = partitioner->getParsedInput();

    // Instantiate the NetList
    netList = new NetList(input);

    // Get a grid, only need to do this once since it is static
    backgroundGrid = netList->generateGridGeometries();
    
    // Create our render window object
    // Give it a default type (titlebar, close button)
    sf::RenderWindow window(sf::VideoMode(
        static_cast<unsigned int>(WIN_VIEWPORT_WIDTH),
        static_cast<unsigned int>(WIN_VIEWPORT_HEIGHT)),
        "Partitioner", sf::Style::Titlebar | sf::Style::Close | sf::Style::Resize);
    window.setView(calcView(window.getSize(), viewportSize));

    lastTime = clock();

    // Do partitioning and output results
    while(window.isOpen())
    {   
        sf::Event event;
        while(window.pollEvent(event))
        {
            if(event.type == sf::Event::Closed)
                window.close();
            if(event.type == sf::Event::Resized)
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
        for(i = 0; i < backgroundGrid.size(); i++)
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
        switch (runMode)
        {
            // Partitioner runs every graphic update
            case EVERY_TICK:
                partitioner->doPartitioning(*netList);
                break;
            // Partitioner runs for a percentage of the total nodes
            case EVERY_NODE_PERCENT:
                for (i = 0; i < netList->getNumNodes()*0.2; i++)
                {
                    partitioner->doPartitioning(*netList);
                }
                break;
            // Partioner for a period of some seconds
            case EVERY_PERIOD:
                if (clock() - lastTime > PARTITIONER_DELAY_PERIOD_MS)
                {
                    partitioner->doPartitioning(*netList);
                    lastTime = clock();
                }
        }
    }

    return 0;
}