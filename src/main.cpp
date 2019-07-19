// clang-format off
#include <unistd.h>
#include <iostream>
#include <utility>
#include <vector>
#include "./AutonomicFarm/Farm.hpp"
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

int main(int argc, char* argv[]) {
    int nWorker = atoi(argv[1]);

    Farm<int>
        f = Farm<int>(nWorker, fib);
    f.start();
    return 0;
}