#include <vector>
#include <unordered_map>
#include <string>
#include <fstream>
#include <iostream>
#include <format>
#include <ranges>
#include <string_view>

struct Node
{
    std::unordered_map<std::string, Node*> htAdj;

    Node()
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
        colonyMap.htNodes[curColonyName] = new Node();
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

int main()
{
    Graph graph;
    readColonyMap("../../tests/hiveum_map_small.txt", graph);
    std::print(std::cout, "Program finished succesfully!");
    return 0;
}