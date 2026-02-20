#include "utils.h"
#pragma once

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