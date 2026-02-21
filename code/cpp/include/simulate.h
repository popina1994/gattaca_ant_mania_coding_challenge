#include <ant.h>
#include <graph.h>

#pragma once

// reset()
// simulate()
//
template <GraphType graphType, NodeType nodeType>
class Simulation
{
    Graph<graphType, nodeType> colonyMapOriginal;
    ColoniesState<nodeType> coloniesStateOriginal;
    ColoniesState<nodeType> coloniesState;
    Graph<graphType, nodeType> colonyMap;

    RandomGenerator randomGenerator;
    uint32_t seed;

    using ColonyIdType =
            std::conditional_t <
            GraphNamesVector<graphType>,
            uint32_t, std::string>;


    static void initializeAntsInner(const Graph<graphType, nodeType>& colonyMap,
    ColoniesState<nodeType>& coloniesState, uint32_t seed)
    {
        std::vector<ColonyIdType> vColonyNames;
        if constexpr (GraphAdjacencyListOpt<graphType>)
        {
            auto keyView = colonyMap.vNodes;
            for (const auto& node: colonyMap.vNodes)
            {
                vColonyNames.push_back(node.colonyName);
            }
        }
        else
        {
            auto keyView = colonyMap.htNodes | std::views::keys;
            vColonyNames.assign(keyView.begin(), keyView.end());
        }

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


    static void collisionHandler(Graph<graphType, nodeType>& colonyMap,
        ColoniesState<nodeType>& coloniesState)
    {
        // vector of vectors is problematic when there is a small number of ants
        // and large number of colonies.
        // The constructor preallocates memory.
        std::unordered_map<ColonyIdType, std::vector<int>> htAntPos(coloniesState.aliveAnts);
        auto aliveAnts = coloniesState.vAnts | std::ranges::views::filter([](const Ant<nodeType>& ant){
                return ant.alive;
        });
        for (const auto& ant: aliveAnts)
        {
            htAntPos[ant.colonyName].push_back(ant.id);
        }
        for (const auto& [colonyName, ants]: htAntPos)
        {
            if (ants.size() > 1) [[unlikely]]
            {
                std::string_view colonyNameStr;
                if constexpr (GraphNamesVector<graphType>)
                {
                    colonyNameStr = colonyMap.vColonyNames[colonyName];
                }
                else
                {
                    colonyNameStr = colonyName;
                }

                if constexpr (GraphAdjacencyListOpt<graphType>)
                {
                    colonyMap.vNodes[colonyName].colonyAlive = false;
                }
                else if constexpr (graphType == GraphType::GraphAdjacencyListMapNameNode)
                {
                    colonyMap.htNodes[colonyName]->colonyAlive = false;
                }
                else if constexpr (graphType == GraphType::GraphAdjacencyListMapNameNodeOpt)
                {
                    colonyMap.htNodes.erase(colonyName);
                }

                CUSTOM_LOG("FIGHTING DESTRUCTION: ");
                for (auto antId: ants)
                {
                    coloniesState.vAnts[antId].alive = false;
                    CUSTOM_LOG(antId, ";");
                }
                CUSTOM_LOG_ENDL;
                CUSTOM_LOG("COLONY ", colonyNameStr, " DESTROYED");
                CUSTOM_LOG_ENDL;
                coloniesState.aliveAnts -= ants.size();
            }
        }
    }


    static void antsMove(const Graph<graphType, nodeType>& colonyMap, ColoniesState<nodeType>& coloniesState, RandomGenerator& randomGenerator)
    {
        auto aliveAnts = coloniesState.vAnts | std::ranges::views::filter([](Ant<nodeType>& ant){
                return ant.alive;
        });

        std::vector<ColonyIdType> vPotentialColonies;
        // Reserve capacity to prevent allocations.
        vPotentialColonies.reserve(GlobalConsts::MAX_NUM_DIRS);
        for (auto& ant: aliveAnts)
        {
            vPotentialColonies.clear();
            if constexpr (nodeType == NodeType::NodeNaive || nodeType == NodeType::NodeOptAdjEdges)
            {
                // htAdj doesn't have an order for keys, while
                // vector of pairs always has a fixed order, thus
                // these two will not produce the order. Is there
                // a defaultdict in C++?
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
            else if constexpr (GraphAdjacencyListOpt<graphType>)
            {
                for (const auto& adjColony: colonyMap.vNodes[ant.colonyName].vAdj)
                {
                    if (adjColony != nullptr && adjColony->colonyAlive)
                    {
                        vPotentialColonies.push_back(adjColony->colonyName);
                    }
                }
            }
            else if constexpr (graphType == GraphType::GraphAdjacencyListMapNameNodeOpt)
            {
                for (auto& adjColony: colonyMap.htNodes.at(ant.colonyName)->vAdj)
                {
                    if (adjColony != nullptr && colonyMap.htNodes.contains(adjColony->colonyName))
                    {
                        vPotentialColonies.push_back(adjColony->colonyName);
                    }
                    else
                    {
                        adjColony = nullptr;
                    }
                }
            }

            if (vPotentialColonies.empty())
            {
                if constexpr (GraphNamesVector<graphType>)
                {
                    CUSTOM_LOG(ant.id, " ISOLATED ", colonyMap.vColonyNames[ant.colonyName]);
                }
                else if constexpr (GraphNamesMap<graphType>)
                {
                    CUSTOM_LOG(ant.id, " ISOLATED ", ant.colonyName);
                }
                CUSTOM_LOG_ENDL
                continue;
            }
            int nextColonyIdx = randomGenerator.generateDir(vPotentialColonies.size());
            ColonyIdType prevColony = ant.colonyName;
            // Spans instead of copies.
            std::string_view prevColonyStr;
            std::string_view curColonyStr;
            if constexpr (FlatNodeType<nodeType>)
            {
                prevColonyStr = colonyMap.vColonyNames[prevColony];
                curColonyStr = colonyMap.vColonyNames[vPotentialColonies[nextColonyIdx]];
            }
            else
            {
                prevColonyStr = ant.colonyName;
                curColonyStr = vPotentialColonies[nextColonyIdx];
            }
            CUSTOM_LOG(ant.id, " MOVED FROM ", prevColonyStr, " TO ", curColonyStr);
            ant.colonyName = vPotentialColonies[nextColonyIdx];
        }
        CUSTOM_LOG_ENDL;
    }


    static void simulateInner(Graph<graphType, nodeType>& colonyMap, ColoniesState<nodeType>& coloniesState, RandomGenerator& randomGenerator)
    {
        static constexpr uint32_t NUM_STEPS = 10'000;
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

