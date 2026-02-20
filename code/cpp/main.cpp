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

enum class  NodeType
{
    NodeNaive,
    NodeOptAdjEdges,
    NodeOptAdjEdgesNameRemoved
};

template <NodeType nodeType>
struct Node;

template <>
struct Node<NodeType::NodeNaive>
{
    using SelfType = Node<NodeType::NodeNaive>;
    std::unordered_map<std::string, SelfType*> htAdj;
    bool colonyAlive = true;
    std::string colonyName;

    Node(const std::string& _colonyName): colonyName(_colonyName)
    {

    }
};

template <>
struct Node<NodeType::NodeOptAdjEdges>
{
    using SelfType = Node<NodeType::NodeOptAdjEdges>;
    static constexpr uint32_t MAX_NUM_DIRS = 4;
    std::array<std::pair<std::string, SelfType*>, MAX_NUM_DIRS> htAdj;
    bool colonyAlive = true;
    std::string colonyName;

    Node(const std::string& _colonyName): colonyName(_colonyName)
    {

    }
};

template <>
struct Node<NodeType::NodeOptAdjEdgesNameRemoved>
{
    using SelfType = Node<NodeType::NodeOptAdjEdgesNameRemoved>;
    static constexpr uint32_t MAX_NUM_DIRS = 4;
    std::array<SelfType*, MAX_NUM_DIRS> vAdj{};
    bool colonyAlive = true;
    std::string colonyName;

    Node(const std::string& _colonyName): colonyName(_colonyName)
    {
    }
};

template<NodeType nodeType>
struct Graph
{
    std::unordered_map<std::string, Node<nodeType>*> htNodes;

    Graph() = default;

    void copyNode(const Graph& other)
    {
        for (const auto& [nodeName, node]: other.htNodes)
        {
            htNodes[nodeName] = new Node<nodeType>(node->colonyName);
        }

        if constexpr (nodeType == NodeType::NodeNaive || nodeType == NodeType::NodeOptAdjEdges)
        {
            for (const auto& [nodeName, node]: other.htNodes)
            {
                if constexpr (nodeType == NodeType::NodeNaive)
                {
                    for (const auto& [dir, adjNode]: node->htAdj)
                    {
                        htNodes[nodeName]->htAdj[dir] = htNodes[adjNode->colonyName];
                    }
                }
                else if constexpr (nodeType == NodeType::NodeOptAdjEdges)
                {
                    int idx = 0;
                    for (const auto& [dirName, adjNode]: node->htAdj)
                    {
                        if (adjNode == nullptr) {continue;}
                        htNodes[nodeName]->htAdj[idx] = {dirName, htNodes[adjNode->colonyName]};
                        idx++;
                    }
                }
            }
        }
        else if constexpr (nodeType == NodeType::NodeOptAdjEdgesNameRemoved)
        {
            for (const auto& [nodeName, node]: other.htNodes)
            {
                int idx = 0;
                for (const auto& adjNode: node->vAdj)
                {
                    if (adjNode == nullptr) {continue;}
                    htNodes[nodeName]->vAdj[idx] = htNodes[adjNode->colonyName];
                    idx++;
                }
            }
        }
    }

    Graph(const Graph& other)
    {
        copyNode(other);
    }

    Graph& operator=(const Graph& other)
    {
        for (auto [nodeName, node]: htNodes)
        {
            delete node;
        }
        htNodes.clear();

        copyNode(other);

        return *this;
    }

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

template<NodeType nodeType>
__attribute__((no_instrument_function))
bool readColonyMap(const std::string& colonyMapFileName, Graph<nodeType>& colonyMap)
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
        colonyMap.htNodes[curColonyName] = new Node<nodeType>(curColonyName);
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
    static const std::unordered_map<std::string, int> dirToIdx = {{"north", 0}, {"south", 1}, {"east", 2}, {"west", 3}};

    for (auto& [curColonyName, vAdjDirColonyNames]: htDirColony)
    {
        for (auto& [adjColonyDir, adjColonyName]: vAdjDirColonyNames)
        {
            if constexpr (nodeType == NodeType::NodeNaive)
            {
                colonyMap.htNodes[curColonyName]->htAdj[adjColonyDir] = colonyMap.htNodes[adjColonyName];
            }
            else if constexpr (nodeType == NodeType::NodeOptAdjEdges)
            {
                int dirIdx = dirToIdx.at(adjColonyDir);
                colonyMap.htNodes[curColonyName]->htAdj[dirIdx] = {adjColonyName, colonyMap.htNodes[adjColonyName]};
            }
            else if constexpr (nodeType == NodeType::NodeOptAdjEdgesNameRemoved)
            {
                int dirIdx = dirToIdx.at(adjColonyDir);
                colonyMap.htNodes[curColonyName]->vAdj[dirIdx] = colonyMap.htNodes[adjColonyName];
            }
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
void antsMove(Graph<nodeType>& colonyMap, ColoniesState& coloniesState, RandomGenerator& randomGenerator)
{
    auto aliveAnts = coloniesState.vAnts | std::ranges::views::filter([](Ant& ant){
            return ant.alive;
    });

    for (auto& ant: aliveAnts)
    {
        std::vector<std::string> vPotentialColonies;
        if constexpr (nodeType == NodeType::NodeNaive || nodeType == NodeType::NodeOptAdjEdges)
        {
            for (const auto& [adjDir, adjColony]: colonyMap.htNodes[ant.colonyName]->htAdj)
            {
                if constexpr (nodeType == NodeType::NodeNaive)
                {
                    if (colonyMap.htNodes[adjColony->colonyName]->colonyAlive)
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
            for (const auto& adjColony: colonyMap.htNodes[ant.colonyName]->vAdj)
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

int main(int argc, char* argv[])
{
    uint32_t seed;
    uint32_t numAnts;

    std::string colonyFileName;


    if (!parseArguments(argc, argv, seed, numAnts, colonyFileName))
    {
        return 1;
    }
    // evaluateExperiment<NodeType::NodeNaive>(seed, numAnts, colonyFileName);
    // evaluateExperiment<NodeType::NodeOptAdjEdges>(seed, numAnts, colonyFileName);
    evaluateExperiment<NodeType::NodeOptAdjEdgesNameRemoved>(seed, numAnts, colonyFileName);

    std::print(std::cout, "Program finished succesfully!");
    return 0;
}