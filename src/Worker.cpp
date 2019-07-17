#include <unistd.h>
#include <functional>
#include <iostream>
#include <mutex>
#include <thread>
#include <typeinfo>
#include <utility>
#include <vector>
#include "./safe_queue.h"

template <class T, class U>
class Worker {
   private:
    std::function<void()> function;
    SafeQueue<T> *inputQueue;
    std::thread workerThread;

   public:
    Worker(std::function<void()> fun, SafeQueue<T> *inputQueue) {
        this->function = fun;
        this->inputQueue = inputQueue;
    }

    void start() {
        std::cout << "worker avviato" << std::endl;
        this->workerThread = std::thread([=] {
            while (true) {
                int x = inputQueue->safe_pop();
                if (x == -1) break;
                std::cout << x << std::endl;
            }
        });

        this->workerThread.join();
    }
};