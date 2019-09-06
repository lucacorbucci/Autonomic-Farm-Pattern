/*
    author: Luca Corbucci
    student number: 516450
*/
// clang-format off
#include <unistd.h>
#include <iostream>
#include <utility>
#include <vector>
#include "../AFP-FFQueue/AutonomicFarm.hpp"
#include "../FastFlow/AutonomicFarm.hpp"
#include "../AFP-SafeQueue/AutonomicFarm.hpp"
#include "../Utils/cxxopts.hpp"
#include "../Utils/parser.hpp"
#include <mutex>
#include <cmath>
#include <chrono>
#include <ff/ubuffer.hpp>
// clang-format on

using namespace ff;

template <class T>
std::vector<T> inputArray{};

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
void init(int nWorker, int tsGoal, int inputSize, U input1, U input2, U input3, int time, std::function<T(U x)> function, bool safeQueue, bool fastFlow, std::string debug) {
    // Fill the vector with input task

    if (fastFlow) {
        AutonomicFarmFF<T, U> f = AutonomicFarmFF<T, U>(nWorker, tsGoal, inputSize, input1, input2, input3, function, time, debug);
        f.start();
    } else {
        // Create the farm
        if (safeQueue) {
            // SafeQueue
            AutonomicFarmSQ<T, U> f = AutonomicFarmSQ<T, U>(nWorker, function, tsGoal, inputSize, input1, input2, input3, time, debug);
            f.start();
        } else {
            // Fastflow queue
            AutonomicFarm<T, U> f = AutonomicFarm<T, U>(nWorker, function, tsGoal, inputSize, input1, input2, input3, time, debug);
            f.start();
        }
    }
}

int main(int argc, char* argv[]) {
    std::stringstream ss;
    ss << argv[0] << " nWorker tsGoal inputSize n1 n2 n3 time";
    cxxopts::Options options("Autonomic Farm patter", ss.str());

    std::string safeQueueString = "false";
    std::string fastFlowString = "false";
    std::string debug = "false";
    bool safeQueue = false;
    bool fastFlow = false;

    // clang-format off
        options.add_options()
            ("h, help", "Help")
            ("f, fastflow", "Fastflow (default: false)", cxxopts::value(fastFlowString))            
            ("d, debug", "Debug (default: false)", cxxopts::value(debug))            
            ("s, safeQueue", "safeQueue (default: false)", cxxopts::value(safeQueueString));
    // clang-format on

    auto result = options.parse(argc, argv);
    if (result.count("help")) {
        std::cout << options.help({"", "Group"}) << std::endl;
        exit(0);
    }
    auto arguments = result.arguments();

    if (argc >= 8) {
        if (result.count("f")) {
            if (result.count("c") || result.count("s")) {
                std::cout << "L'opzione FastFlow va usata da sola senza le altre due" << std::endl;
                return 0;
            }
        }
        if (atoi(argv[1]) <= 0 || atoi(argv[2]) <= 0 || atoi(argv[3]) <= 0 || atoi(argv[7]) <= 0) {
            std::cout << "Paramero non valido" << std::endl;
            std::cout << options.help({"", "Group"}) << std::endl;
            return -1;
        }
        const unsigned int maxWorkers = std::thread::hardware_concurrency();
        int workers = atoi(argv[1]);
        if (workers > maxWorkers)
            workers = maxWorkers;
        try {
            safeQueue = parser(safeQueueString);
            fastFlow = parser(fastFlowString);
        } catch (NonValidValue& e) {
            std::cout << "Paramero non valido" << std::endl;
            std::cout << options.help({"", "Group"}) << std::endl;
            return -1;
        }

        init<int, int>(workers, atoi(argv[2]), atoi(argv[3]), atoi(argv[4]), atoi(argv[5]), atoi(argv[6]), atoi(argv[7]), fib, safeQueue, fastFlow, debug);
    } else {
        std::cout << options.help({"", "Group"}) << std::endl;
    }
    return 0;
}