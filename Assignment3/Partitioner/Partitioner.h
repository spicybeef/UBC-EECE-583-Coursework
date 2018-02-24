#pragma once

#include "Types.h"
#include "NetList.h"

class Partitioner
{
public:
    Partitioner();
    ~Partitioner();

    bool parseInputFile();
    parsedInputStruct_t getParsedInput();
    void doPartitioning(NetList &netList);
    std::string getInfoportString();

    std::string mFilename;              ///< Current filename
    parsedInputStruct_t mParsedInput;   ///< Parsed input struct
    clock_t mStartTime;                 ///< start time for partitioning
    clock_t mEndTime;                   ///< end time for partitioning
    state_e mState;                     ///< Current partitioning state
    int mCurrentGain;                   ///< Keep track of the current gain
    unsigned int mCurrentCutSize;       ///< Keep track of the current cut size
    unsigned int mBestCutSize;                   ///< Keep track of the best cut size so far
    unsigned int mCurrentIteration;     ///< Current partitioner iteration

private:
    const unsigned int MAX_ITERATIONS = 6;
};

