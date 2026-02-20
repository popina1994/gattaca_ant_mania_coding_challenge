#include "ant.h"
#include "graph.h"

struct ColoniesState
{
    std::vector<Ant> vAnts;
    int aliveAnts;

    ColoniesState& operator=(const ColoniesState& other) = default;

    ColoniesState(int numAnts): aliveAnts(numAnts), vAnts(numAnts)
    {}
};

struct RandomGenerator
{
    std::mt19937 gen;
    std::uniform_real_distribution<double> dist;
    RandomGenerator(int seed): gen(seed), dist(0.0, 1.0)
    {}

    int generateDir(int numOutcomes)
    {
        return int(dist(gen) * numOutcomes);
    }
};


template<NodeType nodeType>
__attribute__((no_instrument_function))
void initializeAnts(const Graph<nodeType>& colonies, ColoniesState& coloniesState, int seed)
{
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

template<NodeType nodeType>
void collisionHandler(Graph<nodeType>& colonyMap, ColoniesState& coloniesState)
{
    std::unordered_map<std::string, std::vector<int>> htAntPos;
    auto aliveAnts = coloniesState.vAnts | std::ranges::views::filter([](const Ant& ant){
            return ant.alive;
    });
    for (const auto& ant: aliveAnts)
    {
        htAntPos[ant.colonyName].push_back(ant.id);
    }

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

template<NodeType nodeType>
void antsMove(const Graph<nodeType>& colonyMap, ColoniesState& coloniesState, RandomGenerator& randomGenerator)
{
    auto aliveAnts = coloniesState.vAnts | std::ranges::views::filter([](Ant& ant){
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

template <NodeType nodeType>
void simulate(Graph<nodeType>& colonyMap, ColoniesState& coloniesState, RandomGenerator& randomGenerator)
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

template <NodeType nodeType>
void evaluateExperiment(uint32_t seed, uint32_t numAnts,
        const std::string& colonyFileName)
{
    uint32_t numRepetitions = 100;
    Graph<nodeType> colonyMapOriginal;
    ColoniesState coloniesStateOriginal(numAnts);

    readColonyMap(colonyFileName, colonyMapOriginal);
    initializeAnts(colonyMapOriginal, coloniesStateOriginal, seed);

    for (int idx = 0; idx < numRepetitions; idx ++)
    {
        RandomGenerator randomGenerator(seed);
        ColoniesState coloniesState = coloniesStateOriginal;
        Graph colonyMap = colonyMapOriginal;
        std::println(std::cout, "Iteration: {}", idx);

        auto start = std::chrono::high_resolution_clock::now();
        simulate(colonyMap, coloniesState, randomGenerator);

        auto end = std::chrono::high_resolution_clock::now();
        auto diff = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
        std::println(std::cout, "Precision Time: {} ns ({} ms)",
                    diff.count(),
                    diff.count() / 1'000'000.0);

        std::println(std::cout, "Number of alive ants {}", coloniesState.aliveAnts);
    }
}

bool parseArguments(int argc, char* argv[],
    uint32_t& seed, uint32_t& numAnts, uint32_t& optimizationLevel,
    std::string& fileName)
{
    if (argc != 5) {
        std::cerr << "Usage: " << argv[0] << " <seed> <num_ants> <optimization_level> <filename>\n";
        return false;
    }

    try {
        seed = std::stoul(argv[1]);
        numAnts = std::stoul(argv[2]);
        optimizationLevel = std::stoul(argv[3]);
        fileName = argv[4];
    }
    catch (const std::exception& e) {
        std::cerr << "Error parsing arguments: " << e.what() << "\n";
        return false;
    }
    return true;
}

int main(int argc, char* argv[])
{
    uint32_t seed;
    uint32_t numAnts;
    uint32_t optimizationLevel;

    std::string colonyFileName;


    if (!parseArguments(argc, argv, seed, numAnts, optimizationLevel, colonyFileName))
    {
        return 1;
    }
    switch (optimizationLevel)
    {
        case 0:
        evaluateExperiment<NodeType::NodeNaive>(seed, numAnts, colonyFileName);
            break;
        case 1:
        evaluateExperiment<NodeType::NodeOptAdjEdges>(seed, numAnts, colonyFileName);
            break;
        case 2:
        evaluateExperiment<NodeType::NodeOptAdjEdgesNameRemoved>(seed, numAnts, colonyFileName);
            break;
        default:
            std::cerr << "Invalid optimization level: " << optimizationLevel << "\n";
            return 1;
    }

    std::print(std::cout, "Program finished successfully!");

    return 0;
}