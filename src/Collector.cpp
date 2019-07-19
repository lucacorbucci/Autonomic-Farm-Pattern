// clang-format off
#include <unistd.h>
#include <iostream>
#include <utility>
#include <vector>
#include <boost/lockfree/queue.hpp>
// clang-format on

template <class T>
class Collector {
   private:
    boost::lockfree::queue<int> *inputQueue;
    std::thread collectorThread;
    int activeWorkers;
    std::vector<int> accumulator;

   public:
    Collector(boost::lockfree::queue<int> *inputQueue, int activeWorkers) {
        this->inputQueue = inputQueue;
        this->activeWorkers = activeWorkers;
    }

    void start() {
        std::cout << "Collector avviato" << std::endl;
        this->collectorThread = std::thread([=] {
            int counter = 0;
            while (true) {
                int x;
                if (inputQueue->pop(x)) {
                    if (x == -1) {
                        counter++;
                        if (counter == this->activeWorkers) break;
                    } else {
                        accumulator.push_back(x);
                    }
                }
            }
        });
    }

    void join() {
        this->collectorThread.join();
    }
};