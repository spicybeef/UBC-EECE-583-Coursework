#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <SFML/Graphics.hpp>

#include "SAPlacer.h"

parsedInputStruct_t *input = new parsedInputStruct_t();
placerStruct_t *placer = new placerStruct_t();

int main(int argc, char **argv)
{
    unsigned int i, j;
    char * filename = argv[1];
    //const char * filename = "..\\benchmarks\\kuma.infile";

    // Filename to read in is the second argument
    std::ifstream myfile(filename, std::ios::in);

    // Check if file was opened properly
    if(myfile.is_open())
    {
        printf("File %s opened! Here's what's in it:\n", filename);
    }
    else
    {
        printf("FATAL ERROR, file %s couldn't be opened!\n", filename);
        return -1;
    }

    // Parse input file
    ParseInputFile(&myfile, input);

    // Create our render window object
    // Give it a size of 1024x768
    // Give it a default type (titlebar, close button, resizeable)
    sf::RenderWindow window(sf::VideoMode(1024, 768),
        "Simulated Annealing Placer", sf::Style::Default);

    //sf::Font font;
    //font.loadFromFile("C:/Windows/Fonts/Arial.ttf");

    //sf::Text text;
    //text.setFont(font);
    //text.setPosition(200, 200);
    //text.setString("Hello SFML");

    std::vector<sf::RectangleShape> grid;
    for(i = 0; i < input->numCols; i++)
    {
        for(j = 0; j < input->numRows; j++)
        {
            grid.push_back(sf::RectangleShape());
            grid.back().setPosition((i*(1024 / input->numCols) + 10), (j*(768 / input->numRows) + 10));
            grid.back().setSize(sf::Vector2f(10, 10));
            grid.back().setFillColor(sf::Color::White);
        }
    }

    while(window.isOpen())
    {
        sf::Event event;
        while(window.pollEvent(event))
        {
            if(event.type == sf::Event::Closed)
                window.close();
        }
        window.clear();

        // Draw UI
        // Draw grid
        for(i = 0; i < grid.size(); i++)
        {
            window.draw(grid[i]);
        }
        window.display();
    }

    return 0;
}

std::vector<std::string> SplitString(std::string inString, char delimiter)
{
    std::vector<std::string> internal;
    std::stringstream ss(inString); // Turn the string into a stream.
    std::string temp;

    while(std::getline(ss, temp, delimiter))
    {
        internal.push_back(temp);
    }

    return internal;
}

bool ParseInputFile(std::ifstream *inputFile, parsedInputStruct_t *inputStruct)
{
    unsigned int i, j, numNodes;
    std::string line;
    std::vector<std::string> stringVec;

    // 1. Get number of cells, number of nets, and grid size
    std::getline(*inputFile, line);
    stringVec = SplitString(line, ' ');
    inputStruct->numCells = stoi(stringVec[0]);
    inputStruct->numConnections = stoi(stringVec[1]);
    inputStruct->numRows = stoi(stringVec[2]);
    inputStruct->numCols = stoi(stringVec[3]);
    printf("Grid size is %d rows x %d cols\n", inputStruct->numRows, inputStruct->numCols);
    printf("Number of cells is %d\n", inputStruct->numCells);
    printf("Number of connections is %d\n", inputStruct->numConnections);

    // 2. Get all connections
    for(i = 0; i < inputStruct->numConnections; i++)
    {
        std::getline(*inputFile, line);
        stringVec = SplitString(line, ' ');
        // Get number of nodes for this net
        numNodes = stoi(stringVec[0]);
        printf("Connection %d: %d nodes:\n\t", i, numNodes);
        // Now get all nodes for this net
        // Push back a new vector for this
        inputStruct->nets.push_back(std::vector<unsigned int>());
        for(j = 0; j < numNodes; j++)
        {
            inputStruct->nets[i].push_back(stoi(stringVec[j + 1]));
            printf("%d ", inputStruct->nets[i][j]);
        }
        printf("\n");
    }


    return true;
}