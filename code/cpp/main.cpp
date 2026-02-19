#include <vector>
#include <unordered_map>
#include <string>
#include <fstream>
#include <iostream>
#include <format>
#include <ranges>
#include <string_view>
#include <random>

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
        std::print(std::cout, "CUR COLONY NAME {}", curColonyName);

        subRangesIt ++;
        for (; subRangesIt != splitRange.end(); subRangesIt++)
        {
            auto splitString = std::ranges::to<std::string>(*subRangesIt);
            std::print(std::cout, " NAME {}", splitString);
            htDirColony[curColonyName].push_back(splitDirAndColonyName(splitString));
        }
        std::println(std::cout, "");
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
    std::vector<std::uniform_int_distribution<>> vDistributions;
    static constexpr int NUM_DIRS = 4;
    RandomGenerator(int seed): gen(seed), vDistributions(4)
    {
        int idx = 0;
        for (auto& distribution: vDistributions)
        {
            distribution = std::uniform_int_distribution<> {0, idx};
            idx++;
        }
    }

    int generateDir(int numOutcomes)
    {
        return vDistributions[numOutcomes - 1](gen);
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
            std::cout << "FIGHTING DESTRUCTION: ";
            for (auto antId: ants)
            {
                coloniesState.vAnts[antId].alive = false;
                std::cout << antId << ";";
            }
            std::cout << std::endl;
            std::cout << "COLONY " << colonyName << " DESTROYED" << std::endl;
            coloniesState.aliveAnts -= ants.size();
        }
    }
}

void antsMove(Graph& colonyMap, ColoniesState& coloniesState, RandomGenerator& randomGenerator)
{
    auto aliveAnts = coloniesState.vAnts | std::ranges::views::filter([](const Ant& ant){
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
            std::cout << ant.id << " ISOLATED " << ant.colonyName << std::endl;
            continue;
        }
        int nextColonyIdx = randomGenerator.generateDir(vPotentialColonies.size());
        std::string prevColony = ant.colonyName;
        ant.colonyName = vPotentialColonies[nextColonyIdx];
        std::cout << ant.id << " MOVED FROM " << prevColony << " TO " << ant.colonyName << std::endl;
    }
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

    for (int step = 0; step < NUM_STEPS && coloniesState.aliveAnts > 0; step++)
    {
        std::cout << "STEP " << step << std::endl;
        collisionHandler(colonyMap, coloniesState);
        antsMove(colonyMap, coloniesState, randomGenerator);
        std::cout << std::endl;
    }
    collisionHandler(colonyMap, coloniesState);
    std::cout << "Number of alive ants " << coloniesState.aliveAnts << std::endl;

    std::print(std::cout, "Program finished succesfully!");
    return 0;
}