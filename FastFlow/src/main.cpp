// clang-format off
#include <unistd.h>
#include <iostream>
#include <utility>
#include <vector>
#include <cstdio>
#include <math.h>
#include "./Components/AutonomicFarm.hpp"
// clang-format on

int fib(int x) {
    if ((x == 1) || (x == 0)) {
        return (x);
    } else {
        return (fib(x - 1) + fib(x - 2));
    }
}

int main(int argc, char *argv[]) {
    if (argc >= 6) {
        int nWorker = atoi(argv[1]);
        int tsGoal = atoi(argv[2]);
        int inputSize = atoi(argv[3]);
        int input1 = atoi(argv[4]);
        int input2 = atoi(argv[5]);
        int input3 = atoi(argv[6]);

        AutonomicFarm af{nWorker, tsGoal, inputSize, input1, input2, input3, fib};
        af.start();

    } else {
        std::cout << argv[0] << " Usage: nWorker, tsGoal" << std::endl;
    }
    return 0;
}