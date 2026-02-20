#include "utils.h"
#include "node.h"
#pragma once

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
