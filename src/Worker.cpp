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
// clang-format on

template <class T, class U>
class Worker {
   private:
    std::function<int(int x)> function;
    boost::lockfree::spsc_queue<int> *inputQueue;
    SafeQueue<T> *outputQueue;
    std::thread *workerThread;

   public:
    std::condition_variable *sleepCondition;
    std::mutex *d_mutex;
    std::unique_lock<std::mutex> *lock;
    Worker(std::function<int(int x)> fun, boost::lockfree::spsc_queue<int> *inputQueue, SafeQueue<T> *outputQueue) {
        this->function = fun;
        this->inputQueue = inputQueue;
        this->outputQueue = outputQueue;
        d_mutex = new std::mutex();
        lock = new std::unique_lock<std::mutex>(*d_mutex);
    }

    int fib(int x) {
        if ((x == 1) || (x == 0)) {
            return (x);
        } else {
            return (fib(x - 1) + fib(x - 2));
        }
    }

    void start() {
        std::cout << "worker avviato" << std::endl;
        this->workerThread = new std::thread([=] {
            while (true) {
                // std::cout << std::this_thread::get_id() << std::endl;
                int x;
                inputQueue->pop(x);

                if (x == -1) {
                    // std::cout << "ricevuto -1" << std::endl;
                    break;
                } else {
                    fib(40);
                }
            }
        });
    }

    void join() {
        this->workerThread->join();
        // std::cout << "joinato worker " << std::this_thread::get_id() << std::endl;
    }
};