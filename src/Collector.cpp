// clang-format off
#include <unistd.h>
#include <iostream>
#include <utility>
#include <vector>
// clang-format on

template <class T>
class Collector {
   private:
    SafeQueue<T> *inputQueue;
    std::thread collectorThread;

   public:
    Collector(SafeQueue<T> *inputQueue) {
        this->inputQueue = inputQueue;
    }

    void start() {
        std::cout << "Collector avviato" << std::endl;
        this->collectorThread = std::thread([=] {
            int counter = 0;
            while (true) {
                int x = inputQueue->safe_pop();
                if (x == -1) {
                    counter++;
                    if (counter == 4) break;
                }
            }
        });
    }

    void join() {
        this->collectorThread.join();
    }
};