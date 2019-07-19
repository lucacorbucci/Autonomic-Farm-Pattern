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
#include <boost/lockfree/queue.hpp>
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
    std::vector<boost::lockfree::spsc_queue<Task>*> inputQueues;
    boost::lockfree::queue<Task>* outputQueue = new boost::lockfree::queue<Task>();
    std::vector<Worker<int, int>> workerQueue;

    for (int i = 0; i < nWorker; i++) {
        inputQueues.push_back(new boost::lockfree::spsc_queue<Task>{std::numeric_limits<unsigned int>::max()});
    }

    Emitter<int> e{inputQueues};
    e.start();

    //
    for (int i = 0; i < nWorker; i++) {
        workerQueue.push_back(Worker<int, int>{fib, inputQueues[i], outputQueue});
    }

    for (int i = 0; i < nWorker; i++) {
        workerQueue[i].start();
    }

    Collector<int> c{outputQueue, nWorker};
    c.start();

    for (int i = 0; i < nWorker; i++) {
        workerQueue[i].join();
    }

    e.join();
    c.join();

    return 0;
}