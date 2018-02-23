#pragma once

// typedef helpers to make things legible
typedef std::vector<std::vector<unsigned int>>  netVec;

// partitioning states enum
typedef enum
{
    STATE_START = 0,
    STATE_PARTITIONING,
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
    void doPartitioning();
    std::string getInfoportString();

    std::string mFilename;              ///< Current filename
    parsedInputStruct_t mParsedInput;   ///< Parsed input struct
    clock_t mStarttime;                 ///< start time for partitioning
    clock_t mEndtime;                   ///< end time for partitioning
    state_e mCurrentState;              ///< Current partitioning state
};

