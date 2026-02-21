#include "utils.h"
#include "node.h"
#pragma once

enum class  GraphType
{
    GraphAdjacencyListMapNameNode,
    GraphAdjacencyListMapNameNodeOpt,
    GraphAdjacencyListVectorNameNode,
};

template<GraphType graphType>
concept GraphNamesMap = graphType == GraphType::GraphAdjacencyListMapNameNode;


template<GraphType graphType>
concept GraphNamesVector = (graphType == GraphType::GraphAdjacencyListVectorNameNode) || (graphType == GraphType::GraphAdjacencyListMapNameNodeOpt);


template<GraphType graphType>
concept GraphAdjacencyList = graphType == GraphType::GraphAdjacencyListMapNameNode || graphType == GraphType::GraphAdjacencyListMapNameNodeOpt;

template<GraphType graphType>
concept GraphAdjacencyListOpt = graphType == GraphType::GraphAdjacencyListVectorNameNode;



template<GraphType graphType, NodeType nodeType>
struct Graph;


template<NodeType nodeType>
requires NonFlatNodeType<nodeType>
struct Graph<GraphType::GraphAdjacencyListMapNameNode, nodeType>
{
    static constexpr uint32_t MAX_NUM_DIRS = GlobalConsts::MAX_NUM_DIRS;

    static constexpr GraphType selfGraphType =
    GraphType::GraphAdjacencyListMapNameNode;
    std::unordered_map<std::string, Node<nodeType>*> htNodes;

    Graph() = default;

    void copyNode(const Graph<selfGraphType, nodeType>& other)
    {
        for (const auto& [nodeName, node]: other.htNodes)
        {
            htNodes[nodeName] = new Node<nodeType>(node->colonyName);
        }


        if constexpr (nodeType == NodeType::NodeNaive || nodeType == NodeType::NodeOptAdjEdges)
        {
            for (const auto& [nodeName, node]: other.htNodes)
            {
                *htNodes[nodeName] = *node;
                if constexpr (nodeType == NodeType::NodeNaive)
                {
                    for (const auto& [dir, adjNode]: node->htAdj)
                    {
                        htNodes[nodeName]->htAdj[dir] = htNodes[adjNode->colonyName];
                    }
                }
                else if constexpr (nodeType == NodeType::NodeOptAdjEdges)
                {
                    for (uint32_t idx = 0; idx < MAX_NUM_DIRS; idx++)
                    {
                        auto [dir, adjNode] = node->htAdj[idx];
                        if (adjNode == nullptr) {continue;}
                        htNodes[nodeName]->htAdj[idx] = {dir, htNodes[adjNode->colonyName]};
                    }
                }
            }
        }
        else if constexpr (nodeType == NodeType::NodeOptAdjEdgesNameRemoved)
        {
            for (const auto& [nodeName, node]: other.htNodes)
            {
                *htNodes[nodeName] = *node;
                for (uint32_t idx = 0; idx < MAX_NUM_DIRS; idx++)
                {
                    const auto& adjNode = node->vAdj[idx];
                    if (adjNode == nullptr) {continue;}
                    htNodes[nodeName]->vAdj[idx] = htNodes[adjNode->colonyName];
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

    uint32_t getNumColonies(void)
    {
        return htNodes.size();
    }

    ~Graph()
    {
        for (auto [nodeName, node]: htNodes)
        {
            delete node;
        }
    }
};


template<NodeType nodeType>
requires FlatNodeType<nodeType>
struct Graph<GraphType::GraphAdjacencyListMapNameNodeOpt, nodeType>
{
    static constexpr uint32_t MAX_NUM_DIRS = GlobalConsts::MAX_NUM_DIRS;

    static constexpr GraphType selfGraphType =
    GraphType::GraphAdjacencyListMapNameNodeOpt;
    std::unordered_map<uint32_t, Node<nodeType>*> htNodes;
    std::vector<std::string> vColonyNames;

    Graph() = default;

    void copyNode(const Graph<selfGraphType, nodeType>& other)
    {
        for (const auto& [nodeName, node]: other.htNodes)
        {
            htNodes[nodeName] = new Node<nodeType>(node->colonyName);
        }
        vColonyNames = other.vColonyNames;

        uint32_t idxNode = 0;
        for (const auto& [nodeName, node]: other.htNodes)
        {
            *htNodes[nodeName] = *node;
            for (uint32_t idx = 0; idx < MAX_NUM_DIRS; idx++)
            {
                const auto& adjNode = node->vAdj[idx];
                if (node->vAdj[idx] == nullptr) {continue;}
                htNodes[nodeName]->vAdj[idx] = htNodes[adjNode->colonyName];
            }
            idxNode++;
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

    uint32_t getNumColonies(void)
    {
        return htNodes.size();
    }

    ~Graph()
    {
        for (auto [nodeName, node]: htNodes)
        {
            delete node;
        }
    }
};



template<NodeType flatNodeType>
requires FlatNodeType<flatNodeType>
struct Graph<GraphType::GraphAdjacencyListVectorNameNode, flatNodeType>
{
    static constexpr GraphType selfGraphType =
    GraphType::GraphAdjacencyListVectorNameNode;
    std::vector<Node<flatNodeType>> vNodes;
    std::vector<std::string> vColonyNames;
    static constexpr uint32_t MAX_NUM_DIRS = GlobalConsts::MAX_NUM_DIRS;
    Graph() = default;

    void copyNode(const Graph<selfGraphType, flatNodeType>& other)
    {
        vNodes.resize(other.vNodes.size());
        for (size_t i = 0; i < other.vNodes.size(); i++)
        {
            // copies all fields including pointers, but we will overwrite pointers
            vNodes[i] = other.vNodes[i];
        }
        vColonyNames = other.vColonyNames;
        uint32_t idxNode = 0;
        for (const auto& node: other.vNodes)
        {
            for (uint32_t idx = 0; idx < MAX_NUM_DIRS; idx++)
            {
                const auto& adjNode = node.vAdj[idx];
                if (node.vAdj[idx] == nullptr) {continue;}
                vNodes[idxNode].vAdj[idx] = &vNodes[adjNode->colonyName];
            }
            idxNode++;
        }
    }

    Graph(const Graph& other)
    {
        copyNode(other);
    }

    Graph& operator=(const Graph& other)
    {
        vNodes.clear();
        vColonyNames.clear();

        copyNode(other);

        return *this;
    }

    uint32_t getNumColonies(void)
    {
        return vNodes.size();
    }

    ~Graph()
    {
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

template<GraphType graphType, NodeType nodeType>
__attribute__((no_instrument_function))
bool readColonyMap(const std::string& colonyMapFileName, Graph<graphType, nodeType>& colonyMap)
{
    std::ifstream colonyMapFile(colonyMapFileName);
    if (!colonyMapFile) {
        std::cerr << std::format("File {} does not exist", colonyMapFileName);
        return false;
    }

    std::string line;
    std::unordered_map<std::string,
        std::vector<std::pair<std::string, std::string>>> htDirColony;
    std::unordered_map<std::string, uint32_t> htColonyNameToIdx;

    for (uint32_t cntLine = 0; std::getline(colonyMapFile, line); cntLine++)
    {
        auto splitRange = line | std::views::split(' ');
        auto subRangesIt = splitRange.begin();
        std::string curColonyName = std::ranges::to<std::string>(*subRangesIt);
        if constexpr (GraphNamesVector<graphType>)
        {
            colonyMap.vColonyNames.push_back(curColonyName);
            htColonyNameToIdx[curColonyName] = cntLine;
        }
        if constexpr (graphType == GraphType::GraphAdjacencyListMapNameNode)
        {
            colonyMap.htNodes[curColonyName] = new Node<nodeType>(curColonyName);
        }
        else if constexpr (graphType == GraphType::GraphAdjacencyListMapNameNodeOpt)
        {
            colonyMap.htNodes[cntLine] = new Node<nodeType>(cntLine);
        }
        else if constexpr (graphType == GraphType::GraphAdjacencyListVectorNameNode)
        {
            colonyMap.vNodes.push_back(Node<nodeType>(cntLine));
        }
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

    static const std::unordered_map<std::string, uint32_t> dirToIdx = {{"north", 0}, {"south", 1}, {"east", 2}, {"west", 3}};
    if constexpr (GraphNamesVector<graphType>)
    {
        uint32_t idxColony = 0;
        for (const auto& curColonyName: colonyMap.vColonyNames)
        {
            for (const auto& [adjColonyDir, adjColonyName]: htDirColony[curColonyName])
            {

                uint32_t dirIdx = dirToIdx.at(adjColonyDir);
                if constexpr (graphType == GraphType::GraphAdjacencyListVectorNameNode)
                {
                    colonyMap.vNodes[idxColony].vAdj[dirIdx] = &colonyMap.vNodes[htColonyNameToIdx[adjColonyName]];
                }
                else if constexpr (graphType == GraphType::GraphAdjacencyListMapNameNodeOpt)
                {

                    colonyMap.htNodes[idxColony]->vAdj[dirIdx] = colonyMap.htNodes[htColonyNameToIdx[adjColonyName]];
                }
                CUSTOM_LOG("Setting adj for ", curColonyName, " in direction ", adjColonyDir, " ", adjColonyName);
            }
            idxColony++;
        }
    }
    else
    {
        for (const auto& [curColonyName, vAdjDirColonyNames]: htDirColony)
        {
            for (const auto& [adjColonyDir, adjColonyName]: vAdjDirColonyNames)
            {
                if constexpr (nodeType == NodeType::NodeNaive)
                {
                    colonyMap.htNodes[curColonyName]->htAdj[adjColonyDir] = colonyMap.htNodes[adjColonyName];
                }
                else if constexpr (nodeType == NodeType::NodeOptAdjEdges)
                {
                    uint32_t dirIdx = dirToIdx.at(adjColonyDir);
                    colonyMap.htNodes[curColonyName]->htAdj[dirIdx] = {adjColonyDir, colonyMap.htNodes[adjColonyName]};
                }
                else if constexpr (nodeType == NodeType::NodeOptAdjEdgesNameRemoved)
                {
                    uint32_t dirIdx = dirToIdx.at(adjColonyDir);
                    colonyMap.htNodes[curColonyName]->vAdj[dirIdx] = colonyMap.htNodes[adjColonyName];
                }
                CUSTOM_LOG("Setting adj for ", curColonyName, " in direction ", adjColonyDir, " ", adjColonyName);
            }
        }
    }

    return true;
}
