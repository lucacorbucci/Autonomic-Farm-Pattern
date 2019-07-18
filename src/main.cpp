// clang-format off
#include <unistd.h>
#include <iostream>
#include <utility>
#include <vector>
#include "Worker.cpp"
#include "Emitter.cpp"
#include "Collector.cpp"
#include <mutex>
#include "utimer.hpp"
#include <boost/lockfree/spsc_queue.hpp>
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
    std::vector<boost::lockfree::spsc_queue<int>*> inputQueues;
    SafeQueue<int> outputQueue;
    std::vector<Worker<int, int>> workerQueue;

    for (int i = 0; i < nWorker; i++) {
        inputQueues.push_back(new boost::lockfree::spsc_queue<int>{std::numeric_limits<unsigned int>::max()});
    }

    // Emitter
    Emitter<int> e{inputQueues};
    e.start();

    // Worker
    for (int i = 0; i < nWorker; i++) {
        workerQueue.push_back(Worker<int, int>{fib, inputQueues[i], &outputQueue});
    }

    for (int i = 0; i < nWorker; i++) {
        workerQueue[i].start();
    }

    // Collector
    // Collector<int> c{&outputQueue};
    // c.start();

    for (int i = 0; i < nWorker; i++) {
        workerQueue[i].join();
    }

    e.join();

    return 0;
}