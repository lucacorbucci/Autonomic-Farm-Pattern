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
#include <ff/ubuffer.hpp>
#include <ff/mpmc/MPMCqueues.hpp>

// clang-format on
using namespace ff;

#define WORKING true;
#define STOPPED false;

///  @brief Implementation of the Worker of the autonomic farm
template <class T, class U>
class Worker {
   private:
    std::function<int(int x)> function;
    SWSR_Ptr_Buffer *inputQueue;
    uMPMC_Ptr_Queue *outputQueue;
    std::thread *workerThread;
    std::condition_variable *waitCondition;
    std::mutex *d_mutex;
    std::unique_lock<std::mutex> *lock;
    bool status = true;

    ///  @brief Put this thread in sleep
    ///  @return Void
    void sleep() {
        this->waitCondition->wait(*lock);
    }

    ///  @brief Wake up a sleeping worker
    ///  @details
    ///  This function pop an item from the input queue.
    ///
    ///  @return Returns -1 if it is the last item of the queue and i have to kill
    ///  the worker. In this case i send a Task with value -1 to the collector.
    ///  Returns 0 if it's not the last item. In this case i send the task that i've popped
    ///  from the queue to the collector.
    int compute() {
        void *tmpTask;
        if (!inputQueue->empty()) {
            if (inputQueue->pop(&tmpTask)) {
                Task *t = reinterpret_cast<Task *>(tmpTask);

                //std::cout << "Worker 2 " << t->value << std::endl;

                //std::cout << "Worker 2 " << t->value << std::endl;
               
                
                t->startingTime = std::chrono::high_resolution_clock::now();
                if (t->value == -1) {
                    outputQueue->push(t);
                    return -1;
                } else {
                    //std::cout << std::this_thread::get_id() << std::endl;
                   // std::cout << t.value << std::endl;

                    this->function(t->value);
                    t->endingTime = std::chrono::high_resolution_clock::now();
                    outputQueue->push(t);
                } 
            }
        }
        return 0;
    }

   public:
    ///  @brief Constructor method of the Worker component
    ///  @param fun          The function to be computed
    ///  @param inputQueue   The queue from which the worker extract the task to be computed
    ///  @param outputQueue  The queue where the worker push the computed task
    Worker(std::function<int(int x)> fun, SWSR_Ptr_Buffer *inputQueue, uMPMC_Ptr_Queue *outputQueue) {
        this->function = fun;
        this->inputQueue = inputQueue;
        this->outputQueue = outputQueue;
        d_mutex = new std::mutex();
        lock = new std::unique_lock<std::mutex>(*d_mutex);
        waitCondition = new std::condition_variable();
        this->status = true;
    }

    ///  @brief This function starts the Worker and computes the task
    ///  @return Void
    void start() {
        //std::cout << "worker avviato" << std::endl;
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

    ///  @brief Stop a worker
    ///  @return False if the function works correctely
    bool stopWorker() {
        this->status = false;
        return this->status;
    }
    ///  @brief This function Checks if the worker is active
    ///  @return True if the worker is active, False if the worker is sleeping
    bool isActive() {
        return this->status;
    }

    ///  @brief This function restart a sleeping thread
    ///  @return True if the function works correctely
    bool restartWorker() {
        this->status = true;
        this->waitCondition->notify_one();
        return this->status;
    }

    ///  @brief This function is used to join the emitter thread
    ///  @return Void
    void join() {
        this->workerThread->join();
    }
};