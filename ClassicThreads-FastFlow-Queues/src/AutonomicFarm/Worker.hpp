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
#include "../Utils/Task.hpp"
#include "../Utils/Feedback.hpp"
#include <ff/ubuffer.hpp>
#include <ff/mpmc/MPMCqueues.hpp>

// clang-format on
using namespace ff;

#define WORKING true;
#define STOPPED false;

///  @brief Implementation of the Worker of the autonomic farm
///  @detail Typename T is used for as output type of the function that
///  the worker will compute. Typename U is input as output type of the function
///  that the worker will compute.
template <class T, class U>
class Worker {
   private:
    std::function<T(U x)> function;
    SWSR_Ptr_Buffer *inputQueue;
    uMPMC_Ptr_Queue *outputQueue;
    std::thread *workerThread;
    std::condition_variable *waitCondition;
    std::mutex *d_mutex = new std::mutex();
    std::unique_lock<std::mutex> *lock = new std::unique_lock<std::mutex>(*d_mutex);
    bool status = true;
    int ID;

    

    ///  @brief Put this thread in sleep
    ///  @return Void
    void sleep() {
        this->waitCondition->wait(*lock);
    }

    ///  @brief Debug function used to print some useful informations
    void debug(Task<T, U> *t) {
        std::chrono::duration<double> elapsed = t->endingTime - t->startingTime;
        int elapsedINT = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
        std::cout << "elapsed " << elapsedINT << " current number of workers " << t->workingThreads << " current worker " << std::this_thread::get_id() << std::endl;
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
                Task<T, U> *t = reinterpret_cast<Task<T, U> *>(tmpTask);

                t->startingTime = std::chrono::high_resolution_clock::now();
                if (t->end == -1) {
                    outputQueue->push(t);
                    return -1;
                } else {
                    t->result = this->function(t->value);
                    t->endingTime = std::chrono::high_resolution_clock::now();
                    //debug(t);
                    outputQueue->push(t);
                    return 1;
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
    Worker(int ID, std::function<T(U x)> fun, SWSR_Ptr_Buffer *inputQueue, uMPMC_Ptr_Queue *outputQueue) {
        this->function = fun;
        this->inputQueue = inputQueue;
        this->outputQueue = outputQueue;
        waitCondition = new std::condition_variable();
        this->status = true;
        this->ID = ID;
    }

    ///  @brief This function starts the Worker and computes the task
    ///  @return Void
    void start() {
        //std::cout << "worker avviato" << std::endl;
        this->workerThread = new std::thread([=] {
            int exit = 0;
            while (compute() != -1) {
                while (!isActive()) {
                    sleep();
                }
                if (exit == 1) break;
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

    int printID() {
        return this->ID;
    }
};