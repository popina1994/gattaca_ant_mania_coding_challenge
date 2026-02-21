#include "utils.h"
#include "node.h"
#pragma once


struct AntBase
{
    uint32_t id;
    bool alive = true;
    AntBase(uint32_t _id): id(_id), alive(true) {}
};

template<NodeType nodeType>
struct Ant;

template<NodeType nodeType>
struct Ant: public AntBase
{
    std::string colonyName;

    static uint32_t glId;

    Ant(): AntBase(glId++) {}
};


template<NodeType nodeType>
uint32_t Ant<nodeType>::glId = 0;


template<>
struct Ant<NodeType::NodeOptColonyNamesInts>: public AntBase {
    uint32_t colonyName;

    static uint32_t glId;

    Ant(): AntBase(glId++) {}
};

uint32_t Ant<NodeType::NodeOptColonyNamesInts>::glId = 0;


template<>
struct alignas(std::hardware_destructive_interference_size)
Ant<NodeType::NodeOptColonyNamesIntsAligned> : public AntBase {
    uint32_t colonyName;

    static uint32_t glId;

    Ant(): AntBase(glId++) {}
};

uint32_t Ant<NodeType::NodeOptColonyNamesIntsAligned>::glId = 0;


enum class ColoniesStateType
{
    AntsVector,
    AntsUnorderedMap
};

struct ColoniesStateBase
{
    uint32_t aliveAnts = 0;
    uint32_t numAnts;

    ColoniesStateBase(uint32_t _numAnts = 0): numAnts(_numAnts) {}


};

template<ColoniesStateType coloniesStateType, NodeType nodeType>
struct ColoniesState;

template<NodeType nodeType>
struct ColoniesState<ColoniesStateType::AntsVector, nodeType>: public ColoniesStateBase
{
    using ColonyIdType =
        std::conditional_t <
        FlatNodeType<nodeType>,
        uint32_t, std::string>;

    std::vector<Ant<nodeType>> vAnts;

    ColoniesState& operator=(const ColoniesState& other) = default;

    ColoniesState(uint32_t _numAnts = 0): ColoniesStateBase(_numAnts), vAnts(_numAnts)
    {}

    void initializeAnts(const std::vector<typename ColoniesState::ColonyIdType>& vColonyNames, uint32_t seed)
    {
        std::mt19937 gen(seed);
        std::uniform_int_distribution<> dist(0, vColonyNames.size()-1);

        for (auto& ant: vAnts)
        {
            antAlive(ant.id, vColonyNames[dist(gen)]);
        }
    }

    void antAlive(uint32_t antId, ColonyIdType colonyName)
    {
        vAnts[antId].colonyName = colonyName;
        vAnts[antId].alive = true;
        aliveAnts++;
    }

    void antDied(uint32_t antId)
    {
        vAnts[antId].alive = false;
        aliveAnts--;
    }

    auto liveAntsVector()
    {
        auto aliveAntsRange = vAnts | std::ranges::views::filter([](const Ant<nodeType>& ant){
                return ant.alive;
        });
        return aliveAntsRange;
    }
};

template<NodeType nodeType>
struct ColoniesState<ColoniesStateType::AntsUnorderedMap, nodeType> : public ColoniesStateBase
{
    using ColonyIdType =
        std::conditional_t <
        FlatNodeType<nodeType>,
        uint32_t, std::string>;

    std::unordered_map<uint32_t, Ant<nodeType>*> htAnts;

    ColoniesState(uint32_t _numAnts = 0): ColoniesStateBase(_numAnts), htAnts(_numAnts) {}

    ColoniesState(const ColoniesState& other)
    {
        copyFrom(other);
    }

    ColoniesState& operator=(const ColoniesState& other)
    {
        if (this == &other) {return *this;}
        for (auto [antId, ant]: htAnts)
        {
            delete ant;
        }
        htAnts.clear();
        copyFrom(other);

        return *this;
    }

    void copyFrom(const ColoniesState& other)
    {
        aliveAnts = other.aliveAnts;
        numAnts = other.numAnts;
        for (const auto& [antId, ant]: other.htAnts)
        {
            htAnts[antId] = new Ant<nodeType>(*ant);
        }
    }

    void initializeAnts(const std::vector<typename ColoniesState::ColonyIdType>& vColonyNames, uint32_t seed)
    {
        std::mt19937 gen(seed);
        std::uniform_int_distribution<> dist(0, vColonyNames.size()-1);

        for (int idx = 0; idx < numAnts; idx++)
        {
            antAlive(idx, vColonyNames[dist(gen)]);
        }
    }

    void antAlive(uint32_t antId, ColonyIdType colonyName)
    {
        htAnts[antId] = new Ant<nodeType>();
        htAnts[antId]->colonyName = colonyName;
        aliveAnts++;
    }

    void antDied(uint32_t antId)
    {
        auto ant = htAnts[antId];
        htAnts.erase(antId);
        delete ant;
        aliveAnts--;
    }

    auto liveAntsVector()
    {
        auto aliveAntsRange = htAnts | std::views::values | std::views::transform([](Ant<nodeType>* p)->Ant<nodeType>& { return *p; });
        return aliveAntsRange;
    }
};

