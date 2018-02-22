#include <iomanip>
#include <sstream>
#include <ctime>
#include <cstdlib>

#include "Util.h"

void seedRandom()
{
	// Seed our randomizer with the current time
	std::srand(static_cast<unsigned int>(std::time(nullptr)));
}

int getRandomInt(int i)
{
	return std::rand() % i;
}

double getRandomDouble(void)
{
	return static_cast<double>(std::rand()) / static_cast<double>(RAND_MAX);
}

std::vector<std::string> splitString(std::string inString, char delimiter)
{
	std::vector<std::string> internal;
	std::stringstream ss(inString); // Turn the string into a stream.
	std::string temp;

	while (std::getline(ss, temp, delimiter))
	{
		internal.push_back(temp);
	}

	return internal;
}