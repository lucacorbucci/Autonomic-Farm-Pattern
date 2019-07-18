// clang-format off
#include <unistd.h>
#include <iostream>
#include <utility>
#include <vector>
#include <boost/lockfree/spsc_queue.hpp>
// clang-format on

template <class T>
class Emitter {
   private:
    std::vector<boost::lockfree::spsc_queue<int> *> outputQueue;
    std::thread emitterThread;

   public:
    Emitter(std::vector<boost::lockfree::spsc_queue<int> *> outputQueue) {
        this->outputQueue = outputQueue;
    }

    long mod(long a, long b) { return (a % b + b) % b; }

    void start() {
        std::cout << "Emitter avviato" << std::endl;
        this->emitterThread = std::thread([=] {
            int i = 100;
            auto index = 0;
            while (i > 0) {
                index = mod(i, outputQueue.size());
                outputQueue[index]->push(i);
                i--;
            }
            for (int y = 0; y < outputQueue.size(); y++) {
                // std::cout << "emmit -1" << std::endl;
                outputQueue[y]->push(-1);
            }
        });
    }

    void join() {
        this->emitterThread.join();
        // std::cout << "joinato emitter" << std::endl;
    }
};