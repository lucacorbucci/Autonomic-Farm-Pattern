// clang-format off
#include <chrono>
// clang-format on

struct Task {
    int value;
    std::chrono::high_resolution_clock::time_point startingTime;
    std::chrono::high_resolution_clock::time_point endingTime;
};