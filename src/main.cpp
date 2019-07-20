// clang-format off
#include <unistd.h>
#include <iostream>
#include <utility>
#include <vector>
#include "./AutonomicFarm/AutonomicFarm.hpp"
#include <mutex>
#include <chrono>
// clang-format on

int fib(int x) {
    if ((x == 1) || (x == 0)) {
        return (x);
    } else {
        return (fib(x - 1) + fib(x - 2));
    }
}

std::vector<Task> fillVector(int inputSize, int n1, int n2, int n3) {
    std::vector<Task> inputVector;
    inputVector.reserve(inputSize);
    for (int i = 0; i < inputSize; i++) {
        Task task;
        if (i > 2 * (inputSize / 3))
            task.value = n1;
        else if (i > (inputSize / 3)) {
            task.value = n2;
        } else {
            task.value = n3;
        }
        inputVector.push_back(task);
    }
    return inputVector;
}

///  @brief
///
///  @tparam T   Any float-point type such as float, double or long double
///  @param fun  Non-linear function f(x)
///  @param dfun Derivative of non-linear function df(x) = d/dx f(x)
///  @param x0   Initial guess
///  @param eps  Tolerance for stopping criteria.
///  @return     Equation result as a float point type T.
///
///  @details
///
///

///  Example:
///  @code
///
///  @endcode
///
int main(int argc, char* argv[]) {
    if (argc >= 7) {
        int nWorker = atoi(argv[1]);
        int tsGoal = atoi(argv[2]);
        int inputSize = atoi(argv[3]);
        int input1 = atoi(argv[4]);
        int input2 = atoi(argv[5]);
        int input3 = atoi(argv[6]);

        // Fill the vector with input task
        std::vector<Task>
            inputVector = fillVector(inputSize, input1, input2, input3);

        // Create the farm
        AutonomicFarm<int> f = AutonomicFarm<int>(nWorker, fib, tsGoal, inputVector);
        f.start();
    } else {
        std::cout << argv[0] << " Usage: nWorker, tsGoal" << std::endl;
    }

    return 0;
}