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

int main(int argc, char* argv[]) {
    if (argc >= 6) {
        AutonomicFarm<int, int> af{atoi(argv[1]), atoi(argv[2]), atoi(argv[3]), atoi(argv[4]), atoi(argv[5]), atoi(argv[6]), fib};
        //AutonomicFarm<bool, const char*> af{atoi(argv[1]), atoi(argv[2]), atoi(argv[3]), argv[4], argv[5], argv[6], isPalindrome};
        af.start();
    } else {
        std::cout << argv[0] << " Usage: nWorker, tsGoal" << std::endl;
    }
    return 0;
}
