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

namespace GraphConsts
{
    static const std::unordered_map<uint32_t, std::string_view> idxToDir = {{0, "north"}, {1, "south"}, {2, "east"}, {3, "west"}};
    static const std::unordered_map<std::string, uint32_t> dirToIdx = {{"north", 0}, {"south", 1}, {"east", 2}, {"west", 3}};
}


template<NodeType nodeType>
requires NonFlatNodeType<nodeType>
struct Graph<GraphType::GraphAdjacencyListMapNameNode, nodeType>
{
    using ColonyIdType = std::string;
    static constexpr uint32_t MAX_NUM_DIRS = GlobalConsts::MAX_NUM_DIRS;

    static constexpr GraphType selfGraphType =
    GraphType::GraphAdjacencyListMapNameNode;
    std::unordered_map<std::string, Node<nodeType>*> htNodes;

    Graph() = default;

    void copyFrom(const Graph<selfGraphType, nodeType>& other)
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
        copyFrom(other);
    }

    Graph& operator=(const Graph& other)
    {
        if (this == &other) {return *this;}
        for (auto [nodeName, node]: htNodes)
        {
            delete node;
        }
        htNodes.clear();

        copyFrom(other);

        return *this;
    }

    uint32_t getNumColonies(void) const
    {
        return htNodes.size();
    }

    void colonyDeath(const ColonyIdType& colonyName)
    {
        htNodes[colonyName]->colonyAlive = false;
    }

    auto getLiveColonies() const
    {
        return htNodes
        | std::views::values
        | std::views::filter
        ([](const auto* node)
        {
            return node->colonyAlive;
        });
    }

    const std::string_view getColonyName(const Node<nodeType>& node) const
    {
        return node.colonyName;
    }

    const std::string_view getColonyName(const ColonyIdType& colonyId) const
    {
        return colonyId;
    }

    auto adjacentColonies(const ColonyIdType& colonyName) const
    {
        if constexpr (nodeType == NodeType::NodeNaive)
        {
            return htNodes.at(colonyName)->htAdj
            | std::views::values
            | std::views::filter
            ([](const auto* adjColony)
            {
                return adjColony->colonyAlive;
            });
        }
        else if constexpr (nodeType == NodeType::NodeOptAdjEdges)
        {
            return htNodes.at(colonyName)->htAdj
            | std::views::filter
            ([](const auto& pair)
            {
                return pair.second != nullptr && pair.second->colonyAlive;
            })
            | std::views::transform
            ([](const auto& pair)
            {
                return pair.second;
            });
        }
        else if constexpr (nodeType == NodeType::NodeOptAdjEdgesNameRemoved)
        {
            return htNodes.at(colonyName)->vAdj | std::views::filter
            ([](const auto* adjColony)
            {
                return adjColony != nullptr && adjColony->colonyAlive;
            });
        }
    }

    auto adjacentColoniesDir(const ColonyIdType& colonyName) const
    {
        if constexpr (nodeType == NodeType::NodeNaive || nodeType  == NodeType::NodeOptAdjEdges)
        {
            return htNodes.at(colonyName)->htAdj
            | std::views::transform([](const auto& pair) -> std::pair<std::string_view, const Node<nodeType>*>
            {
                return {std::string_view(pair.first), pair.second};
            })
            | std::views::filter([](const auto& pair)
            {
                return pair.second != nullptr && pair.second->colonyAlive;
            });
        }
        else if constexpr (nodeType == NodeType::NodeOptAdjEdgesNameRemoved)
        {
            const auto& idxToDir = GraphConsts::idxToDir;
            return std::views::iota(0u, MAX_NUM_DIRS)
            | std::views::transform([this, colonyName, &idxToDir](uint32_t idx)  -> std::pair<std::string_view, const Node<nodeType>*>
            {
                return {std::string_view(idxToDir.at(idx)), htNodes.at(colonyName)->vAdj[idx]};
            })
            | std::views::filter([this](const auto& pair)
            {
                return pair.second != nullptr && this->htNodes.contains(pair.second->colonyName);
            });
        }
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
    using ColonyIdType = uint32_t;
    static constexpr uint32_t MAX_NUM_DIRS = GlobalConsts::MAX_NUM_DIRS;

    static constexpr GraphType selfGraphType =
    GraphType::GraphAdjacencyListMapNameNodeOpt;
    std::unordered_map<uint32_t, Node<nodeType>*> htNodes;
    std::vector<std::string> vColonyNames;

    Graph() = default;

    void copyFrom(const Graph<selfGraphType, nodeType>& other)
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
        copyFrom(other);
    }

    Graph& operator=(const Graph& other)
    {
        if (this == &other) {return *this;}
        for (auto [nodeName, node]: htNodes)
        {
            delete node;
        }
        htNodes.clear();

        copyFrom(other);

        return *this;
    }

    uint32_t getNumColonies(void) const
    {
        return htNodes.size();
    }

    void colonyDeath(const ColonyIdType& colonyName)
    {
        htNodes.erase(colonyName);
    }

    auto getLiveColonies() const
    {
        return htNodes | std::views::values;
    }

    const std::string_view getColonyName(const Node<nodeType>& node) const
    {
        return vColonyNames[node.colonyName];
    }


    const std::string_view getColonyName(const ColonyIdType& colonyId) const
    {
        return vColonyNames[colonyId];
    }

    auto adjacentColonies(const ColonyIdType& colonyName) const
    {
        return htNodes.at(colonyName)->vAdj
        | std::views::filter
        ([this](const auto* adjColony)
        {
            return adjColony != nullptr && this->htNodes.contains(adjColony->colonyName);
        });
    }

    auto adjacentColoniesDir(const ColonyIdType& colonyName) const
    {
        const auto& idxToDir = GraphConsts::idxToDir;
        return std::views::iota(0u, MAX_NUM_DIRS)
        | std::views::transform([this, colonyName, &idxToDir](uint32_t idx)-> std::pair<std::string_view, const Node<nodeType>*>
        {
            return {std::string_view(idxToDir.at(idx)), htNodes.at(colonyName)->vAdj[idx]};
        })
        | std::views::filter([this](const auto& pair)
        {
            return pair.second != nullptr && this->htNodes.contains(pair.second->colonyName);
        });
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
    using ColonyIdType = uint32_t;
    static constexpr GraphType selfGraphType =
    GraphType::GraphAdjacencyListVectorNameNode;
    std::vector<Node<flatNodeType>> vNodes;
    std::vector<std::string> vColonyNames;
    static constexpr uint32_t MAX_NUM_DIRS = GlobalConsts::MAX_NUM_DIRS;
    Graph() = default;

    void copyFrom(const Graph<selfGraphType, flatNodeType>& other)
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
        copyFrom(other);
    }

    Graph& operator=(const Graph& other)
    {
        if (this == &other) {return *this;}
        vNodes.clear();
        vColonyNames.clear();

        copyFrom(other);

        return *this;
    }

    uint32_t getNumColonies(void) const
    {
        return vNodes.size();
    }

    void colonyDeath(const ColonyIdType& colonyName)
    {
        vNodes[colonyName].colonyAlive = false;
    }

    auto getLiveColonies() const
    {
        return vNodes | std::views::filter([](const auto& node) { return node.colonyAlive; }) | std::views::transform([](auto& node) { return &node; });
    }

    const std::string_view getColonyName(const Node<flatNodeType>& node) const
    {
        return vColonyNames[node.colonyName];
    }

    const std::string_view getColonyName(const ColonyIdType& colonyId) const
    {
        return vColonyNames[colonyId];
    }

    auto adjacentColonies(const ColonyIdType& colonyName) const
    {
        return vNodes[colonyName].vAdj
        | std::views::filter
        ([](const auto* adjColony)
        {
            return adjColony != nullptr && adjColony->colonyAlive;
        });
    }

    auto adjacentColoniesDir(const ColonyIdType& colonyName) const
    {
        const auto& idxToDir = GraphConsts::idxToDir;
        return std::views::iota(0u, MAX_NUM_DIRS)
        | std::views::transform([this, colonyName, &idxToDir](uint32_t idx) -> std::pair<std::string_view, const Node<flatNodeType>*>
        {
            return {std::string_view(idxToDir.at(idx)), vNodes[colonyName].vAdj[idx]};
        })
        | std::views::filter([](const auto& pair)
        {
            return pair.second != nullptr && pair.second->colonyAlive;
        });
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

    const auto& dirToIdx = GraphConsts::dirToIdx;

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
