#include "utils.h"
#include "node.h"
#pragma once


template<NodeType nodeType>
struct Ant;

template<NodeType nodeType>
struct Ant
{
    int id;
    std::string colonyName;
    bool alive;

    static int glId;

    Ant(): id(glId++){}
};


template<NodeType nodeType>
int Ant<nodeType>::glId = 0;


template<>
struct Ant<NodeType::NodeOptColonyNamesInts> {
    int id;
    int colonyName;
    bool alive;

    static int glId;

    Ant() : id(glId++), alive(true) {}
};

int Ant<NodeType::NodeOptColonyNamesInts>::glId = 0;

template<NodeType nodeType>
struct ColoniesState
{
    std::vector<Ant<nodeType>> vAnts;
    int aliveAnts;

    ColoniesState& operator=(const ColoniesState& other) = default;

    ColoniesState(int numAnts = 0): aliveAnts(numAnts), vAnts(numAnts)
    {}
};
