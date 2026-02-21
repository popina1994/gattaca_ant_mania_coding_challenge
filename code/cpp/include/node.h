#include "utils.h"
#include <type_traits>
#pragma once

enum class  NodeType
{
    NodeNaive,
    NodeOptAdjEdges,
    NodeOptAdjEdgesNameRemoved,
    NodeOptColonyNamesInts,
    NodeOptColonyNamesIntsAligned,
};

template<NodeType nodeType>
concept NonFlatNodeType = nodeType == NodeType::NodeNaive || nodeType == NodeType::NodeOptAdjEdgesNameRemoved || nodeType == NodeType::NodeOptAdjEdges;

template<NodeType nodeType>
concept FlatNodeType = (nodeType == NodeType::NodeOptColonyNamesInts) || (nodeType == NodeType::NodeOptColonyNamesIntsAligned);


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
    static constexpr uint32_t MAX_NUM_DIRS = GlobalConsts::MAX_NUM_DIRS;
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
    static constexpr uint32_t MAX_NUM_DIRS = GlobalConsts::MAX_NUM_DIRS;
    std::array<SelfType*, MAX_NUM_DIRS> vAdj{};
    bool colonyAlive = true;
    std::string colonyName;

    Node(const std::string& _colonyName): colonyName(_colonyName)
    {
    }
};

template <>
struct Node<NodeType::NodeOptColonyNamesInts>
{
    using SelfType = Node<NodeType::NodeOptColonyNamesInts>;
    static constexpr uint32_t MAX_NUM_DIRS = GlobalConsts::MAX_NUM_DIRS;
    std::array<SelfType*, MAX_NUM_DIRS> vAdj{};
    bool colonyAlive = true;
    uint32_t colonyName;
    Node(){}
    Node(uint32_t _colonyName): colonyName(_colonyName)
    {
    }
};


template <>
struct alignas(std::hardware_destructive_interference_size)
Node<NodeType::NodeOptColonyNamesIntsAligned>
{
    using SelfType = Node<NodeType::NodeOptColonyNamesIntsAligned>;
    static constexpr uint32_t MAX_NUM_DIRS = GlobalConsts::MAX_NUM_DIRS;
    std::array<SelfType*, MAX_NUM_DIRS> vAdj{};
    bool colonyAlive = true;
    uint32_t colonyName;
    Node(){}
    Node(uint32_t _colonyName): colonyName(_colonyName)
    {
    }
};


