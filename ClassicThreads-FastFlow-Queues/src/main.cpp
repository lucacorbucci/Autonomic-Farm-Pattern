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
template <typename T, typename U>
std::vector<Task<T, U>*> fillVector(int inputSize, int n1, int n2, int n3) {
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

// template <typename U>
// std::vector<Task<T, U>*> fillVector(int inputSize, int n1, int n2, int n3) {
//     std::vector<Task<T, U>*> inputVector;
//     inputVector.reserve(inputSize);
//     for (int i = 0; i < inputSize; i++) {
//         Task<T, U>* task = new Task<T, U>();
//         if (i > 2 * (inputSize / 3))
//             task->value = "accesecarbonimacadedacaminobracesecca";
//         else if (i > (inputSize / 3)) {
//             task->value = "accesecarbonimacadedacaminobracesecca";
//         } else {
//             task->value = "accesecarbonimacadedacaminobracesecca";
//         }
//         inputVector.push_back(task);
//     }
//     return inputVector;
// }

// template <typename U>
// std::vector<Task<T, U>*> fillVector(int inputSize, int n1, int n2, int n3) {
//     std::vector<Task<T, U>*> inputVector;
//     inputVector.reserve(inputSize);
//     for (int i = 0; i < inputSize; i++) {
//         Task<T, U>* task = new Task<T, U>();
//         if (i > 2 * (inputSize / 3))
//             task->value = 179424691;
//         else if (i > (inputSize / 3)) {
//             task->value = 179424691;
//         } else {
//             task->value = 179424691;
//         }
//         inputVector.push_back(task);
//     }
//     return inputVector;
// }

// template <typename T, typename U>
// std::vector<Task<T, U>*> fillVector(int inputSize, int n1, int n2, int n3) {
//     std::vector<Task<T, U>*> inputVector;
//     inputVector.reserve(inputSize);
//     for (int i = 0; i < inputSize; i++) {
//         Task<T, U>* task = new Task<T, U>();
//         if (i > 2 * (inputSize / 3))
//             task->value = 179425027;
//         else if (i > (inputSize / 3)) {
//             task->value = 179425027;
//         } else {
//             task->value = 179425027;
//         }
//         inputVector.push_back(task);
//     }
//     return inputVector;
// }

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

bool isPrime(int n) {
    int limit = sqrt(n);
    for (int i = 2; i <= limit; i++) {
        if (n % i == 0) {
            return false;
        }
    }
    return true;
}

template <typename T, typename U>
void init(int nWorker, int tsGoal, int inputSize, int input1, int input2, int input3, std::function<T(U x)> function) {
    // Fill the vector with input task
    std::vector<Task<T, U>*> inputVector = fillVector<T, U>(inputSize, input1, input2, input3);

    // Create the farm
    AutonomicFarm<T, U> f = AutonomicFarm<T, U>(nWorker, function, tsGoal, inputVector);
    f.start();
}

int main(int argc, char* argv[]) {
    if (argc >= 7) {
        int nWorker = atoi(argv[1]);
        int tsGoal = atoi(argv[2]);
        int inputSize = atoi(argv[3]);
        int input1 = atoi(argv[4]);
        int input2 = atoi(argv[5]);
        int input3 = atoi(argv[6]);
        init<int, int>(atoi(argv[1]), atoi(argv[2]), atoi(argv[3]), atoi(argv[4]), atoi(argv[5]), atoi(argv[6]), fib);
        //init<bool, const char*>(atoi(argv[1]), atoi(argv[2]), atoi(argv[3]), atoi(argv[4]), atoi(argv[5]), atoi(argv[6]), isPalindrome);
        //init<bool, int>(atoi(argv[1]), atoi(argv[2]), atoi(argv[3]), atoi(argv[4]), atoi(argv[5]), atoi(argv[6]), isPrime);

    } else {
        std::cout << argv[0] << " Usage: nWorker, tsGoal" << std::endl;
    }

    return 0;
}