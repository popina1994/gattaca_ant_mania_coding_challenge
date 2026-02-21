#include "simulate.h"

template <NodeType nodeType>
void evaluateExperiment(uint32_t seed, uint32_t numAnts,
        const std::string& colonyFileName)
{
    uint32_t numRepetitions = 100;
    Simulation<nodeType> simulation(numAnts, seed);

    simulation.loadColonyMap(colonyFileName);
    simulation.initializeAnts();

    for (int idx = 0; idx < numRepetitions; idx ++)
    {
        simulation.reset();
        std::println(std::cout, "Iteration: {}", idx);

        auto start = std::chrono::high_resolution_clock::now();
        simulation.simulate();

        auto end = std::chrono::high_resolution_clock::now();
        auto diff = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
        std::println(std::cout, "Precision Time: {} ns ({} ms)",
                    diff.count(),
                    diff.count() / 1'000'000.0);

        std::println(std::cout, "Number of alive ants {}", simulation.getLiveAnts());
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
        case 3:
        evaluateExperiment<NodeType::NodeOptColonyNamesInts>(seed, numAnts, colonyFileName);
        break;
        case 4:
        evaluateExperiment<NodeType::NodeOptColonyNamesIntsAligned>(seed, numAnts, colonyFileName);
        break;
        default:
            std::cerr << "Invalid optimization level: " << optimizationLevel << "\n";
            return 1;
    }

    std::print(std::cout, "Program finished successfully!");

    return 0;
}