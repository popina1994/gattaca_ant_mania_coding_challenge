
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

#pragma once

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