#pragma once

#include <vector>

#include <SFML/Graphics.hpp>

#include "Partitioner.h"

int getRandomInt(int i);
std::vector<std::string> splitString(std::string inString, char delimiter);

sf::View calcView(const sf::Vector2u &windowsize, const sf::Vector2u &designedsize);
