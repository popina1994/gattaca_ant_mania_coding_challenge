#include <ant.h>
#include <graph.h>

#pragma once

template <GraphType graphType, ColoniesStateType coloniesStateType, NodeType nodeType>
class Simulation
{
    Graph<graphType, nodeType> colonyMapOriginal;
    ColoniesState<coloniesStateType, nodeType> coloniesStateOriginal;
    ColoniesState<coloniesStateType, nodeType> coloniesState;
    Graph<graphType, nodeType> colonyMap;

    RandomGenerator randomGenerator;
    uint32_t seed;

    using ColonyIdType =
            std::conditional_t <
            GraphNamesVector<graphType>,
            uint32_t, std::string>;


    static void initializeAntsInner(const Graph<graphType, nodeType>& colonyMap,
    ColoniesState<coloniesStateType, nodeType>& coloniesState, uint32_t seed)
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

        coloniesState.initializeAnts(vColonyNames, seed);
    }


    static void collisionHandler(Graph<graphType, nodeType>& colonyMap,
        ColoniesState<coloniesStateType, nodeType>& coloniesState)
    {
        // vector of vectors is problematic when there is a small number of ants
        // and large number of colonies.
        // The constructor preallocates memory.
        std::unordered_map<ColonyIdType, std::vector<int>> htAntPos(coloniesState.aliveAnts);
        auto aliveAnts = coloniesState.liveAntsVector();

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

                colonyMap.colonyDeath(colonyName);

                CUSTOM_LOG("FIGHTING DESTRUCTION: ");
                for (auto antId: ants)
                {
                    coloniesState.antDied(antId);
                    CUSTOM_LOG(antId, ";");
                }
                CUSTOM_LOG_ENDL;
                CUSTOM_LOG("COLONY ", colonyNameStr, " DESTROYED");
                CUSTOM_LOG_ENDL;
            }
        }
    }


    static void antsMove(const Graph<graphType, nodeType>& colonyMap, ColoniesState<coloniesStateType, nodeType>& coloniesState, RandomGenerator& randomGenerator)
    {
        auto aliveAnts = coloniesState.liveAntsVector();

        std::vector<ColonyIdType> vPotentialColonies;
        // Reserve capacity to prevent allocations.
        vPotentialColonies.reserve(GlobalConsts::MAX_NUM_DIRS);
        for (auto& ant: aliveAnts)
        {
            vPotentialColonies.clear();
             // htAdj doesn't have an order for keys, while
            // vector of pairs always has a fixed order, thus
            // these two will not produce the order.
            auto adjacentColonies = colonyMap.adjacentColonies(ant.colonyName);
            auto adjColNames = adjacentColonies
            | std::views::transform
            ([](auto* node)-> ColonyIdType& {
                return node->colonyName;
            });
            std::ranges::copy(adjColNames, std::back_inserter(vPotentialColonies));

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


    static void simulateInner(Graph<graphType, nodeType>& colonyMap, ColoniesState<coloniesStateType, nodeType>& coloniesState, RandomGenerator& randomGenerator)
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

