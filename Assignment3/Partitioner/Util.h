#pragma once

#include <vector>
#include "Partitioner.h"

void seedRandom();
int getRandomInt(int i);
double getRandomDouble(void);
std::vector<std::string> splitString(std::string inString, char delimiter);