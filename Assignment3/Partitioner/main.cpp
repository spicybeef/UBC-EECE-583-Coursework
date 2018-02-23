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
#include "Partitioner.h"
#include "NetList.h"
#include "Util.h"
#include "main.h"

int main(int argc, char **argv)
{
    unsigned int i, swapCount;
    parsedInputStruct_t input;

    // Viewport size
    const sf::Vector2u viewportSize(
        static_cast<unsigned int>(WIN_VIEWPORT_WIDTH),
        static_cast<unsigned int>(WIN_VIEWPORT_HEIGHT));

    // Drawing components
    std::vector<sf::RectangleShape> backgroundGrid;
    std::vector<sf::RectangleShape> placedCells;
    std::vector<sf::Vertex> netLines;

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
    partitioner->mFilename = argv[1];
    //placer->filename = const_cast<char *>("..\\..\\benchmarks\\cm151a.txt");
    // Parse the input file
    partitioner->parseInputFile();
    input = partitioner->getParsedInput();

    // Instantiate the NetList
    netList = new NetList(input);

    // Get a grid, only need to do this once since it is static
    backgroundGrid = netList->generateGridGeometries();

    // Place the cells at random
    netList->randomizeNodePlacement();

    
    // Create our render window object
    // Give it a default type (titlebar, close button)
    sf::RenderWindow window(sf::VideoMode(
        static_cast<unsigned int>(WIN_VIEWPORT_WIDTH),
        static_cast<unsigned int>(WIN_VIEWPORT_HEIGHT)),
        "Partitioner", sf::Style::Titlebar | sf::Style::Close | sf::Style::Resize);
    window.setView(calcView(window.getSize(), viewportSize));

    // Do partitioning and output results
    swapCount = 0;
    while(window.isOpen())
    { 
        // Run our partitioning
        partitioner->doPartitioning();
        swapCount++;
        // Only update every so often to speed up process
        /*
        if(swapCount < static_cast<unsigned int>(0.01 * static_cast<double>(placer->movesPerTempDec)))
        {
            continue;
        }
        else
        {
            swapCount = 0;
        }
        */

        sf::Event event;
        while(window.pollEvent(event))
        {
            if(event.type == sf::Event::Closed)
                window.close();
            if(event.type == sf::Event::Resized)
                window.setView(calcView(sf::Vector2u(event.size.width, event.size.height), viewportSize));
        }

        // Get net lines
        netLines = netList->generateNetGeometries();
        // Get placed cells
        placedCells = netList->generatePlacedNodeGeometries();

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

        // Draw placed cells
        for (i = 0; i < placedCells.size(); i++)
        {
            window.draw(placedCells[i]);
        }
        // Draw net lines
        window.draw(&netLines.front(), netLines.size(), sf::Lines);

        // Display window
        window.display();
    }

    return 0;
}