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

///  @detail Typename T is used for as output type of the function that
///  the worker will compute. Typename U is input as output type of the function
///  that the worker will compute.
template <typename T, typename U>
void init(int nWorker, int tsGoal, int inputSize, U input1, U input2, U input3, int time, std::function<T(U x)> function, bool collector, bool safeQueue, bool fastFlow, std::string debug) {
    // Fill the vector with input task
    std::vector<Task<T, U>*> inputVector = fillVector<T, U>(inputSize, input1, input2, input3);

    if (fastFlow) {
        AutonomicFarmFF<T, U> f = AutonomicFarmFF<T, U>(nWorker, tsGoal, inputSize, input1, input2, input3, function, time, debug);
        f.start();
    } else {
        // Create the farm
        if (safeQueue) {
            // SafeQueue
            AutonomicFarmSQ<T, U> f = AutonomicFarmSQ<T, U>(nWorker, function, tsGoal, inputVector, collector, time, debug);
            f.start();
        } else {
            // Fastflow queue
            AutonomicFarm<T, U> f = AutonomicFarm<T, U>(nWorker, function, tsGoal, inputVector, collector, time, debug);
            f.start();
        }
    }
}

int main(int argc, char* argv[]) {
    std::stringstream ss;
    ss << argv[0] << "nWorker inputSize tsGoal n1 n2 n3";
    cxxopts::Options options("Autonomic Farm patter", ss.str());

    std::string collectorString = "true";
    std::string safeQueueString = "false";
    std::string fastFlowString = "false";
    std::string debug = "false";

    bool collector = true;
    bool safeQueue = false;
    bool fastFlow = false;
    bool debug = false;

    const unsigned int maxWorkers = std::thread::hardware_concurrency();
    int workers = atoi(argv[1]);

    // clang-format off
        options.add_options()
            ("h, help", "Help")
            ("c, collector", "Collector (default: true)", cxxopts::value(collectorString))
            ("f, fastflow", "Fastflow (default: false)", cxxopts::value(fastFlowString))            
            ("d, debug", "Debug (default: false)", cxxopts::value(debug))            
            ("s, safeQueue", "safeQueue (default: false)", cxxopts::value(safeQueueString));
    // clang-format on

    if (argc >= 8) {
        auto result = options.parse(argc, argv);
        if (result.count("help")) {
            std::cout << options.help({"", "Group"}) << std::endl;
            exit(0);
        }
        auto arguments = result.arguments();

        if (result.count("f")) {
            if (result.count("c") || result.count("s")) {
                std::cout << "L'opzione FastFlow va usata da sola senza le altre due" << std::endl;
                return 0;
            }
        }

        if (workers > maxWorkers)
            workers = maxWorkers;
        collector = parser(collectorString);
        safeQueue = parser(safeQueueString);
        fastFlow = parser(fastFlowString);

        init<int, int>(workers, atoi(argv[2]), atoi(argv[3]), atoi(argv[4]), atoi(argv[5]), atoi(argv[6]), atoi(argv[7]), fib, collector, safeQueue, fastFlow, debug);
    } else {
        std::cout << options.help({"", "Group"}) << std::endl;
    }
    return 0;
}