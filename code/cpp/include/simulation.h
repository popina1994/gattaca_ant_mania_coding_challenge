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
        vColonyNames.reserve(colonyMap.getNumColonies());
        auto colNames = colonyMap.getLiveColonies()
            | std::views::transform
            ([](const auto* node)-> const ColonyIdType& {
                return node->colonyName;
            });

        std::ranges::copy(colNames, std::back_inserter(vColonyNames));

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
                const auto& colonyNameStr = colonyMap.getColonyName(colonyName);

                colonyMap.colonyDeath(colonyName);

                CUSTOM_LOG_NO_SPACE("FIGHTING ANTS DIED: ");
                for (auto antId: ants)
                {
                    coloniesState.antDied(antId);
                    CUSTOM_LOG_NO_SPACE(antId, ";");
                }
                CUSTOM_LOG_ENDL;
                CUSTOM_LOG("COLONY", colonyNameStr, "DESTROYED");
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
                CUSTOM_LOG(ant.id, " ISOLATED ", colonyMap.getColonyName(ant.colonyName));
                continue;
            }
            int nextColonyIdx = randomGenerator.generateDir(vPotentialColonies.size());
            ColonyIdType prevColony = ant.colonyName;
            // Spans instead of copies.
            const auto& prevColonyStr = colonyMap.getColonyName(prevColony);
            const auto& curColonyStr = colonyMap.getColonyName(vPotentialColonies[nextColonyIdx]);
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

    static void printStateInner(Graph<graphType, nodeType>& colonyMap, ColoniesState<coloniesStateType, nodeType>& coloniesState)
    {
        CUSTOM_LOG("PRINTING STATE");
        CUSTOM_LOG_ENDL;
        CUSTOM_LOG("PRINTING STATE ANTS");
        CUSTOM_LOG_ENDL;
        for (const auto& ant: coloniesState.liveAntsVector())
        {
            CUSTOM_LOG("ANT", ant.id, "IS IN COLONY", colonyMap.getColonyName(ant.colonyName));
        }

        CUSTOM_LOG("PRINTING STATE COLONIES");
        CUSTOM_LOG_ENDL;
        for (const auto* colony: colonyMap.getLiveColonies())
        {
            CUSTOM_LOG_NO_SPACE(colonyMap.getColonyName(*colony));
            auto adjacentColonies = colonyMap.adjacentColoniesDir(colony->colonyName);
            for (const auto& [direction, adjColony]: adjacentColonies)
            {
                CUSTOM_LOG_NO_SPACE(" ", direction, "=", colonyMap.getColonyName(*adjColony));
            }
            CUSTOM_LOG_ENDL;
        }
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

    void printState()
    {
        printStateInner(colonyMap, coloniesState);
    }
};

