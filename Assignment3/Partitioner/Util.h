#pragma once

#include <vector>

#include <SFML/Graphics.hpp>

#include "Partitioner.h"

void seedRandom();
int getRandomInt(int i);
double getRandomDouble(void);
std::vector<std::string> splitString(std::string inString, char delimiter);

sf::View calcView(const sf::Vector2u &windowsize, const sf::Vector2u &designedsize);
