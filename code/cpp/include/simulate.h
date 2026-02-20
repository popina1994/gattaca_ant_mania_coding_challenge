#include <ant.h>
#include <graph.h>

#pragma once

// reset()
// simulate()
//
template <NodeType nodeType>
class Simulation
{
    Graph<nodeType> colonyMapOriginal;
    ColoniesState<nodeType> coloniesStateOriginal;
    ColoniesState<nodeType> coloniesState;
    Graph<nodeType> colonyMap;

    RandomGenerator randomGenerator;
    uint32_t seed;


    static void initializeAntsInner(const Graph<nodeType>& colonies,
    ColoniesState<nodeType>& coloniesState, uint32_t seed)
    {
        // CONDITIONAL ...
        // template alias?
        auto keyView = colonies.htNodes | std::views::keys;
        std::vector<std::string> vColonyNames(keyView.begin(), keyView.end());
        int numColonies = vColonyNames.size();

        std::mt19937 gen(seed);
        std::uniform_int_distribution<> dist(0, numColonies-1);

        for (auto& ant: coloniesState.vAnts)
        {
            ant.alive = true;
            int colonyIdx = dist(gen);
            ant.colonyName = vColonyNames[colonyIdx];
        }
    }

    static void collisionHandler(Graph<nodeType>& colonyMap,
        ColoniesState<nodeType>& coloniesState)
    {
        // CONDITIONAL ...
        std::unordered_map<std::string, std::vector<int>> htAntPos;
        auto aliveAnts = coloniesState.vAnts | std::ranges::views::filter([](const Ant<nodeType>& ant){
                return ant.alive;
        });
        for (const auto& ant: aliveAnts)
        {
            htAntPos[ant.colonyName].push_back(ant.id);
        }
        // TODO: constexpr
        for (const auto& [colonyName, ants]: htAntPos)
        {
            if (ants.size() > 1)
            {
                colonyMap.htNodes[colonyName]->colonyAlive = false;
                CUSTOM_LOG("FIGHTING DESTRUCTION: ");
                for (auto antId: ants)
                {
                    coloniesState.vAnts[antId].alive = false;
                    CUSTOM_LOG(antId, ";");
                }
                CUSTOM_LOG_ENDL;
                CUSTOM_LOG("COLONY ", colonyName, " DESTROYED");
                CUSTOM_LOG_ENDL;
                coloniesState.aliveAnts -= ants.size();
            }
        }
    }

    static void antsMove(const Graph<nodeType>& colonyMap, ColoniesState<nodeType>& coloniesState, RandomGenerator& randomGenerator)
    {
        auto aliveAnts = coloniesState.vAnts | std::ranges::views::filter([](Ant<nodeType>& ant){
                return ant.alive;
        });

        for (auto& ant: aliveAnts)
        {
            std::vector<std::string> vPotentialColonies;
            if constexpr (nodeType == NodeType::NodeNaive || nodeType == NodeType::NodeOptAdjEdges)
            {
                for (const auto& [adjDir, adjColony]: colonyMap.htNodes.at(ant.colonyName)->htAdj)
                {
                    if constexpr (nodeType == NodeType::NodeNaive)
                    {
                        if (colonyMap.htNodes.at(adjColony->colonyName)->colonyAlive)
                        {
                            vPotentialColonies.push_back(adjColony->colonyName);
                        }
                    }
                    else if constexpr (nodeType == NodeType::NodeOptAdjEdges)
                    {
                        if (adjColony != nullptr && adjColony->colonyAlive)
                        {
                            vPotentialColonies.push_back(adjColony->colonyName);
                        }
                    }
                }
            }
            else  if constexpr (nodeType == NodeType::NodeOptAdjEdgesNameRemoved)
            {
                for (const auto& adjColony: colonyMap.htNodes.at(ant.colonyName)->vAdj)
                {
                    if (adjColony != nullptr && adjColony->colonyAlive)
                    {
                        vPotentialColonies.push_back(adjColony->colonyName);
                    }
                }
            }

            if (vPotentialColonies.empty())
            {
                CUSTOM_LOG(ant.id, " ISOLATED ", ant.colonyName);
                CUSTOM_LOG_ENDL
                continue;
            }
            int nextColonyIdx = randomGenerator.generateDir(vPotentialColonies.size());
            std::string prevColony = ant.colonyName;
            ant.colonyName = vPotentialColonies[nextColonyIdx];
            CUSTOM_LOG(ant.id, " MOVED FROM ", prevColony, " TO ", ant.colonyName);
        }
        CUSTOM_LOG_ENDL;
    }

    static void simulateInner(Graph<nodeType>& colonyMap, ColoniesState<nodeType>& coloniesState, RandomGenerator& randomGenerator)
    {
        static constexpr uint32_t NUM_STEPS = 10'000;
        // static constexpr uint32_t NUM_STEPS = 100;
        for (int step = 0; step < NUM_STEPS && coloniesState.aliveAnts > 0; step++)
        {
            CUSTOM_LOG("STEP ", step);
            CUSTOM_LOG_ENDL;
            collisionHandler(colonyMap, coloniesState);
            antsMove(colonyMap, coloniesState, randomGenerator);
            CUSTOM_LOG_ENDL;
        }
        collisionHandler(colonyMap, coloniesState);
    }

public:
    Simulation(uint32_t numAnts, uint32_t _seed): coloniesStateOriginal(numAnts),
    seed(_seed), randomGenerator(_seed)
    {
    }

    __attribute__((no_instrument_function))
    void loadColonyMap(const std::string& colonyFileName)
    {
        readColonyMap(colonyFileName, colonyMapOriginal);
    }

    __attribute__((no_instrument_function))
    void initializeAnts()
    {
        initializeAntsInner(colonyMapOriginal, coloniesStateOriginal, seed);
    }

    void reset()
    {
        randomGenerator = RandomGenerator(seed);
        coloniesState = coloniesStateOriginal;
        colonyMap = colonyMapOriginal;
    }

    void simulate()
    {
        simulateInner(colonyMap, coloniesState, randomGenerator);
    }

    uint32_t getLiveAnts()
    {
        return coloniesState.aliveAnts;
    }
};

