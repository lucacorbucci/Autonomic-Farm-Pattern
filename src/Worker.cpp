// clang-format off
#include <unistd.h>
#include <functional>
#include <iostream>
#include <mutex>
#include <thread>
#include <typeinfo>
#include <utility>
#include <vector>
#include <condition_variable>
#include "./safe_queue.h"
#include <math.h>
#include <boost/lockfree/spsc_queue.hpp>
#include <boost/lockfree/queue.hpp>
// clang-format on

template <class T, class U>
class Worker {
   private:
    std::function<int(int x)> function;
    boost::lockfree::spsc_queue<int> *inputQueue;
    boost::lockfree::queue<int> *outputQueue;
    std::thread *workerThread;

   public:
    std::condition_variable *sleepCondition;
    std::mutex *d_mutex;
    std::unique_lock<std::mutex> *lock;
    Worker(std::function<int(int x)> fun, boost::lockfree::spsc_queue<int> *inputQueue, boost::lockfree::queue<int> *outputQueue) {
        this->function = fun;
        this->inputQueue = inputQueue;
        this->outputQueue = outputQueue;
        d_mutex = new std::mutex();
        lock = new std::unique_lock<std::mutex>(*d_mutex);
    }

    void start() {
        std::cout << "worker avviato" << std::endl;
        this->workerThread = new std::thread([=] {
            while (true) {
                int x;
                if (inputQueue->pop(x)) {
                    if (x == -1) {
                        outputQueue->push(-1);
                        break;
                    } else {
                        this->function(40);
                        outputQueue->push(x);
                    }
                }
            }
        });
    }

    void join() {
        this->workerThread->join();
    }
};