// clang-format off
#include <unistd.h>
#include <iostream>
#include <utility>
#include <vector>
#include "Worker.hpp"
#include "Emitter.hpp"
#include "Collector.hpp"
#include <mutex>
#include <boost/lockfree/spsc_queue.hpp>
#include <boost/lockfree/queue.hpp>
#include <chrono>
// clang-format on

class Farm {
   private:
    std::vector<boost::lockfree::spsc_queue<Task>*> inputQueues;
    boost::lockfree::queue<Task>* outputQueue = new boost::lockfree::queue<Task>();
    std::vector<Worker<int, int>> workerQueue;
    int nWorker;
    std::function<int(int x)> function;

   public:
    Farm(int nWorker, std::function<int(int x)> function) {
        this->nWorker = nWorker;
        this->function = function;
    }

    void start() {
        for (int i = 0; i < this->nWorker; i++) {
            this->inputQueues.push_back(new boost::lockfree::spsc_queue<Task>{std::numeric_limits<unsigned int>::max()});
        }

        Emitter<int> e{this->inputQueues};
        e.start();

        //
        for (int i = 0; i < this->nWorker; i++) {
            this->workerQueue.push_back(Worker<int, int>{this->function, this->inputQueues[i], this->outputQueue});
        }

        for (int i = 0; i < this->nWorker; i++) {
            this->workerQueue[i].start();
        }

        Collector<int> c{this->outputQueue, this->nWorker};
        c.start();

        for (int i = 0; i < this->nWorker; i++) {
            this->workerQueue[i].join();
        }

        e.join();
        c.join();
    }
};