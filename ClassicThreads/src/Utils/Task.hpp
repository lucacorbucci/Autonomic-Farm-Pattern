#pragma once
#ifndef TASK_HPP
#define TASK_HPP
// clang-format off
#include <chrono>
// clang-format on

/// @brief Task structure
template <typename T, typename U>
struct Task {
    /// @brief Input value of the function to be computed
    U value;
    int end;
    T result;
    /// @brief Number of currently active threads
    int workingThreads;
    /// @brief Starting time of the task
    std::chrono::high_resolution_clock::time_point startingTime;
    /// @brief Ending time of the task
    std::chrono::high_resolution_clock::time_point endingTime;
};

#endif