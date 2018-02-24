#pragma once

#include "NetList.h"

// typedef helpers to make things legible
typedef std::vector<std::vector<unsigned int>>  netVec;

// partitioning states enum
typedef enum
{
    STATE_INIT = 0,
    STATE_PARTITIONING_START,
    STATE_PARTITIONING_FIND_SWAP_CANDIDATES,
    STATE_PARTITIONING_SWAP_AND_LOCK,
    STATE_FINISHED,
    STATE_NUM
} state_e;

// Parsed input struct
typedef struct
{
    unsigned int                                numRows;                ///< Number of parsed rows
    unsigned int                                numCols;                ///< Number of parsed columns

    unsigned int                                numNodes;               ///< Number of nodes to place
    unsigned int                                numConnections;         ///< The number of connections

    netVec                                      nets;                   ///< Parsed nets

} parsedInputStruct_t;

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
    unsigned int mCurrentIteration;     ///< Current partitioner iteration
};

