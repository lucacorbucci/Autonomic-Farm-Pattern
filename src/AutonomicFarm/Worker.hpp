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
#include <math.h>
#include <boost/lockfree/spsc_queue.hpp>
#include <boost/lockfree/queue.hpp>
#include "../Utils/Task.hpp"
// clang-format on

#define WORKING true;
#define STOPPED false;

template <class T, class U>
class Worker {
   private:
    std::function<int(int x)> function;
    boost::lockfree::spsc_queue<Task> *inputQueue;
    boost::lockfree::queue<Task> *outputQueue;
    std::thread *workerThread;
    std::condition_variable *waitCondition;
    std::mutex *d_mutex;
    std::unique_lock<std::mutex> *lock;
    bool status = true;

   public:
    Worker(std::function<int(int x)> fun, boost::lockfree::spsc_queue<Task> *inputQueue, boost::lockfree::queue<Task> *outputQueue) {
        this->function = fun;
        this->inputQueue = inputQueue;
        this->outputQueue = outputQueue;
        d_mutex = new std::mutex();
        lock = new std::unique_lock<std::mutex>(*d_mutex);
        waitCondition = new std::condition_variable();
        this->status = true;
    }

    void start() {
        std::cout << "worker avviato" << std::endl;
        this->workerThread = new std::thread([=] {
            while (true) {
                this->waitCondition->wait(*lock, [=] { return this->status; });

                Task t;
                if (inputQueue->pop(t)) {
                    t.startingTime = std::chrono::high_resolution_clock::now();
                    if (t.value == -1) {
                        outputQueue->push(t);
                        break;
                    } else {
                        this->function(t.value);
                        t.endingTime = std::chrono::high_resolution_clock::now();
                        outputQueue->push(t);
                    }
                }
            }
        });
    }

    void stopWorker() {
        this->status = false;
    }

    bool getStatus() {
        return this->status;
    }

    void restartWorker() {
        this->status = true;
        //this->waitCondition->notify_one();
    }

    void
    join() {
        this->workerThread->join();
    }
};