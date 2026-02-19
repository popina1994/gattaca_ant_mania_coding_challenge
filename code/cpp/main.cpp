#include <vector>
#include <unordered_map>
#include <string>
#include <fstream>
#include <iostream>
#include <format>
#include <ranges>
#include <string_view>
#include <random>
#include <chrono>

#ifndef LOG_ON
#define LOG_ON false
#endif

template <typename... Args>
void log_internal(const char* file, int line, Args&&... args) {
    std::cout << "[" << file << ":" << line << "] ";

    ((std::cout << std::forward<Args>(args) << " "), ...);

    std::cout << std::endl;
}

template <typename... Args>
void log_internal_clean(Args&&... args) {
    ((std::cout << std::forward<Args>(args) << " "), ...);

    std::cout << std::endl;
}

#if defined(LOG_ON) && LOG_ON
#define CUSTOM_LOG(...) log_internal_clean(__VA_ARGS__)
#define CUSTOM_LOG_ENDL std::cout << std::endl;
#else
#define CUSTOM_LOG(...)
#define CUSTOM_LOG_ENDL
#endif

struct Node
{
    std::unordered_map<std::string, Node*> htAdj;
    bool colonyAlive = true;
    std::string colonyName;

    Node(const std::string& _colonyName): colonyName(_colonyName)
    {

    }
};

struct Graph
{
    std::unordered_map<std::string, Node*> htNodes;

    ~Graph()
    {
        for (auto [nodeName, node]: htNodes)
        {
            delete node;
        }
    }
};

std::pair<std::string, std::string>
splitDirAndColonyName(const std::string& str)
{
    auto splitRange = str | std::views::split('=');
    auto subRangesIt = splitRange.begin();

    std::string adjColonyDirection = std::ranges::to<std::string>(*subRangesIt);
    ++subRangesIt;
    std::string adjColonyName = std::ranges::to<std::string>(*subRangesIt);

    return {adjColonyDirection, adjColonyName};
}

bool readColonyMap(const std::string& colonyMapFileName, Graph& colonyMap)
{
    std::ifstream colonyMapFile(colonyMapFileName);
    if (!colonyMapFile) {
        std::cerr << std::format("File {} does not exist", colonyMapFileName);
        return false;
    }

    std::string line;
    std::unordered_map<std::string,
        std::vector<std::pair<std::string, std::string>>> htDirColony;
    while (std::getline(colonyMapFile, line))
    {
        auto splitRange = line | std::views::split(' ');
        auto subRangesIt = splitRange.begin();
        std::string curColonyName = std::ranges::to<std::string>(*subRangesIt);
        colonyMap.htNodes[curColonyName] = new Node(curColonyName);
        htDirColony[curColonyName] = {};
        CUSTOM_LOG("CUR COLONY NAME", curColonyName);

        subRangesIt ++;
        for (; subRangesIt != splitRange.end(); subRangesIt++)
        {
            auto splitString = std::ranges::to<std::string>(*subRangesIt);
            CUSTOM_LOG("NAME ", splitString);
            htDirColony[curColonyName].push_back(splitDirAndColonyName(splitString));
        }
        CUSTOM_LOG_ENDL;
    }

    for (auto& [curColonyName, vAdjDirColonyNames]: htDirColony)
    {
        for (auto& [adjColonyDir, adjColonyName]: vAdjDirColonyNames)
        {
            colonyMap.htNodes[curColonyName]->htAdj[adjColonyDir] = colonyMap.htNodes[adjColonyName];
        }
    }

    return true;
}

struct Ant
{
    int id;
    std::string colonyName;
    bool alive;

    static int glId;

    Ant(): id(glId++){}
};

int Ant::glId = 0;

struct ColoniesState
{
    std::vector<Ant> vAnts;
    int aliveAnts;

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

void initializeAnts(const Graph& colonies, ColoniesState& coloniesState, int seed)
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

void collisionHandler(Graph& colonyMap, ColoniesState& coloniesState)
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

void antsMove(Graph& colonyMap, ColoniesState& coloniesState, RandomGenerator& randomGenerator)
{
    auto aliveAnts = coloniesState.vAnts | std::ranges::views::filter([](Ant& ant){
            return ant.alive;
    });

    for (auto& ant: aliveAnts)
    {
        std::vector<std::string> vPotentialColonies;
        for (const auto& [adjDir, adjColony]: colonyMap.htNodes[ant.colonyName]->htAdj)
        {
            if (colonyMap.htNodes[adjColony->colonyName]->colonyAlive)
            {
                vPotentialColonies.push_back(adjColony->colonyName);
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

bool parseArguments(int argc, char* argv[],
    uint32_t& seed, uint32_t& numAnts, std::string& fileName)
{
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <seed> <num_ants> <filename>\n";
        return false;
    }

    try {
        seed = static_cast<int>(std::stoul(argv[1]));
        numAnts = std::stoul(argv[2]);
        fileName = argv[3];
    }
    catch (const std::exception& e) {
        std::cerr << "Error parsing arguments: " << e.what() << "\n";
        return false;
    }
    return true;
}

int main(int argc, char* argv[])
{
    // static constexpr uint32_t NUM_STEPS = 10'000;
    static constexpr uint32_t NUM_STEPS = 100;
    uint32_t seed;
    uint32_t numAnts;
    std::string colonyFileName;

    if (!parseArguments(argc, argv, seed, numAnts, colonyFileName))
    {
        return 1;
    }

    Graph colonyMap;
    ColoniesState coloniesState(numAnts);
    RandomGenerator randomGenerator(seed);

    readColonyMap(colonyFileName, colonyMap);
    initializeAnts(colonyMap, coloniesState, seed);

    auto start = std::chrono::high_resolution_clock::now();
    for (int step = 0; step < NUM_STEPS && coloniesState.aliveAnts > 0; step++)
    {
        CUSTOM_LOG("STEP ", step);
        CUSTOM_LOG_ENDL;
        collisionHandler(colonyMap, coloniesState);
        antsMove(colonyMap, coloniesState, randomGenerator);
        CUSTOM_LOG_ENDL;
    }
    collisionHandler(colonyMap, coloniesState);

    auto end = std::chrono::high_resolution_clock::now();
    auto diff = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    std::println(std::cout, "Precision Time: {} ns ({} ms)",
                 diff.count(),
                 diff.count() / 1'000'000.0);

    std::println(std::cout, "Number of alive ants {}", coloniesState.aliveAnts);
    std::print(std::cout, "Program finished succesfully!");
    return 0;
}