/*
    author: Luca Corbucci
    student number: 516450
*/
// clang-format off
#include <unistd.h>
#include <iostream>
#include <utility>
#include <vector>
#include <cmath>
#include <chrono>
#include "../Utils/utimer.hpp"
// clang-format on

int fib(int x) {
    if ((x == 1) || (x == 0)) {
        return (x);
    } else {
        return (fib(x - 1) + fib(x - 2));
    }
}

std::vector<int> fillVector(int inputSize, int n1, int n2, int n3) {
    std::vector<int> inputVector;
    inputVector.reserve(inputSize);
    for (int i = 0; i < inputSize; i++) {
        if (i > 2 * (inputSize / 3))
            inputVector.push_back(n3);
        else if (i > (inputSize / 3)) {
            inputVector.push_back(n2);
        } else {
            inputVector.push_back(n1);
        }
    }
    return inputVector;
}

int main(int argc, char* argv[]) {
    if (argc >= 4) {
        std::vector<int> inputVector = fillVector(atoi(argv[1]), atoi(argv[2]), atoi(argv[3]), atoi(argv[4]));
        {
            utimer t("Time");
            for (auto x : inputVector) {
                fib(x);
            }
        }
    } else {
        std::cout << "Usage: inputSize n1 n2 n3" << std::endl;
    }
}