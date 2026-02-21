#include "utils.h"
#include "node.h"
#pragma once


template<NodeType nodeType>
struct Ant;

template<NodeType nodeType>
struct Ant
{
    uint32_t id;
    std::string colonyName;
    bool alive;

    static uint32_t glId;

    Ant(): id(glId++){}
};


template<NodeType nodeType>
uint32_t Ant<nodeType>::glId = 0;


template<>
struct Ant<NodeType::NodeOptColonyNamesInts> {
    uint32_t id;
    uint32_t colonyName;
    bool alive;

    static uint32_t glId;

    Ant() : id(glId++), alive(true) {}
};

uint32_t Ant<NodeType::NodeOptColonyNamesInts>::glId = 0;

template<NodeType nodeType>
struct ColoniesState
{
    std::vector<Ant<nodeType>> vAnts;
    uint32_t aliveAnts;

    ColoniesState& operator=(const ColoniesState& other) = default;

    ColoniesState(uint32_t numAnts = 0): aliveAnts(numAnts), vAnts(numAnts)
    {}
};
