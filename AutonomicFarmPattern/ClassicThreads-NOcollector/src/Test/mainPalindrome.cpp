// clang-format off
#include <unistd.h>
#include <iostream>
#include <utility>
#include <vector>
#include "./AutonomicFarm/AutonomicFarm.hpp"
#include <mutex>
#include <cmath>
#include <chrono>
#include <ff/ubuffer.hpp>
// clang-format on

using namespace ff;

int fib(int x) {
    if ((x == 1) || (x == 0)) {
        return (x);
    } else {
        return (fib(x - 1) + fib(x - 2));
    }
}

///  @detail Typename T is used for as output type of the function that
///  the worker will compute. Typename U is input as output type of the function
///  that the worker will compute.
template <typename T, typename U>
std::vector<Task<T, U>*> fillVector(int inputSize, U n1, U n2, U n3) {
    std::vector<Task<T, U>*> inputVector;
    inputVector.reserve(inputSize);
    for (int i = 0; i < inputSize; i++) {
        Task<T, U>* task = new Task<T, U>();
        if (i > 2 * (inputSize / 3))
            task->value = n3;
        else if (i > (inputSize / 3)) {
            task->value = n2;
        } else {
            task->value = n1;
        }
        inputVector.push_back(task);
    }
    return inputVector;
}

bool isPalindrome(const char* s) {
    size_t n = strlen(s);
    if (n == 0)
        return false;

    const char* e = s + n - 1;
    while (s < e)
        if (*s++ != *e--)
            return false;
    return true;
}

///  @detail Typename T is used for as output type of the function that
///  the worker will compute. Typename U is input as output type of the function
///  that the worker will compute.
template <typename T, typename U>
void init(int nWorker, int tsGoal, int inputSize, U input1, U input2, U input3, std::function<T(U x)> function) {
    // Fill the vector with input task
    std::vector<Task<T, U>*> inputVector = fillVector<T, U>(inputSize, input1, input2, input3);

    // Create the farm
    AutonomicFarm<T, U> f = AutonomicFarm<T, U>(nWorker, function, tsGoal, inputVector);
    f.start();
}

int main(int argc, char* argv[]) {
    if (argc >= 7) {
        init<bool, const char*>(atoi(argv[1]), atoi(argv[2]), atoi(argv[3]), argv[4], argv[5], argv[6], isPalindrome);

    } else {
        std::cout << argv[0] << " Usage: nWorker, tsGoal" << std::endl;
    }

    return 0;
}