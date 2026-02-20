#include "utils.h"
#include "node.h"
#pragma once

template<NodeType nodeType>
struct Graph
{
    static constexpr uint32_t MAX_NUM_DIRS = 4;
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
                    for (int idx = 0; idx < MAX_NUM_DIRS; idx++)
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
                for (int idx = 0; idx < MAX_NUM_DIRS; idx++)
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

    ~Graph()
    {
        for (auto [nodeName, node]: htNodes)
        {
            delete node;
        }
    }
};

template<>
struct Graph<NodeType::NodeOptColonyNamesInts>
{
    std::vector<Node<NodeType::NodeOptColonyNamesInts>*> vNodes;
    std::vector<std::string> vColonyNames;
    static constexpr uint32_t MAX_NUM_DIRS = 4;
    Graph() = default;

    void copyNode(const Graph<NodeType::NodeOptColonyNamesInts>& other)
    {
        for (const auto& node: other.vNodes)
        {
            vNodes.push_back(new Node<NodeType::NodeOptColonyNamesInts>(node->colonyName));
        }
        for (const auto& otherColonyName: other.vColonyNames)
        {
            vColonyNames.push_back(otherColonyName);
        }
        int idxNode = 0;
        for (const auto& node: other.vNodes)
        {
            for (int idx = 0; idx < MAX_NUM_DIRS; idx++)
            {
                const auto& adjNode = node->vAdj[idx];
                if (node->vAdj[idx] == nullptr) {continue;}
                vNodes[idxNode]->vAdj[idx] = vNodes[adjNode->colonyName];
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
        for (auto  node: vNodes)
        {
            delete node;
        }
        vNodes.clear();
        vColonyNames.clear();

        copyNode(other);

        return *this;
    }

    ~Graph()
    {
        for (auto node: vNodes)
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
    std::unordered_map<std::string, int> htColonyNameToIdx;

    for (int cntLine = 0; std::getline(colonyMapFile, line); cntLine++)
    {
        auto splitRange = line | std::views::split(' ');
        auto subRangesIt = splitRange.begin();
        std::string curColonyName = std::ranges::to<std::string>(*subRangesIt);
        if constexpr (nodeType == NodeType::NodeOptColonyNamesInts)
        {
            colonyMap.vColonyNames.push_back(curColonyName);
            colonyMap.vNodes.push_back(new Node<NodeType::NodeOptColonyNamesInts>(cntLine));
            htColonyNameToIdx[curColonyName] = cntLine;
        }
        else
        {
            colonyMap.htNodes[curColonyName] = new Node<nodeType>(curColonyName);
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

    static const std::unordered_map<std::string, int> dirToIdx = {{"north", 0}, {"south", 1}, {"east", 2}, {"west", 3}};
    if constexpr (nodeType == NodeType::NodeOptColonyNamesInts)
    {
        int idxColony = 0;
        for (auto& curColonyName: colonyMap.vColonyNames)
        {
            for (auto& [adjColonyDir, adjColonyName]: htDirColony[curColonyName])
            {
                int dirIdx = dirToIdx.at(adjColonyDir);
                colonyMap.vNodes[idxColony]->vAdj[dirIdx] = colonyMap.vNodes[htColonyNameToIdx[adjColonyName]];
            }
            idxColony++;
        }
    }
    else
    {
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
                else if constexpr (nodeType == NodeType::NodeOptColonyNamesInts)
                {
                    int dirIdx = dirToIdx.at(adjColonyDir);
                    colonyMap.vNodes[curColonyName]->vAdj[dirIdx] = colonyMap.htNodes[adjColonyName];
                }
            }
        }
    }

    return true;
}
