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
#include "../Utils/Feedback.hpp"
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

    /*
        This function pop an item from the input queue.
        Returns -1 if it is the last item of the queue and i have to kill
        the worker. In this case i send a Task with value -1 to the collector.
        Returns 0 if it's not the last item. In this case i send the task that i've popped
        from the queue to the collector.
    */
    int compute() {
        Task t;
        if (inputQueue->pop(t)) {
            t.startingTime = std::chrono::high_resolution_clock::now();
            if (t.value == -1) {
                outputQueue->push(t);
                return -1;
            } else {
                //std::cout << std::this_thread::get_id() << std::endl;
                this->function(t.value);
                t.endingTime = std::chrono::high_resolution_clock::now();
                outputQueue->push(t);
            }
        }
        return 0;
    }

    /*
        This function puts the thread in sleep, i've used the condition variable
        and the call wait to block the worker until the emitter decide to wake it up.
    */
    void sleep() {
        this->waitCondition->wait(*lock);
    }

    void start() {
        std::cout << "worker avviato" << std::endl;
        this->workerThread = new std::thread([=] {
            while (true) {
                if (compute() == -1) break;
                if (!isActive()) {
                    // I empty the queue before going to sleep
                    if (compute() == -1) break;
                    sleep();
                }
            }
        });
    }

    bool stopWorker() {
        this->status = false;
        return this->status;
    }

    bool isActive() {
        return this->status;
    }

    bool restartWorker() {
        this->status = true;
        this->waitCondition->notify_one();
        return this->status;
    }

    void join() {
        this->workerThread->join();
    }
};