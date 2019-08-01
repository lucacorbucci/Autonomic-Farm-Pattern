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
    ///  @brief Function to be computed from the worker
    std::function<T(U x)> function;
    ///  @brief Queue from which the worker extract the task sent by the emitter
    SWSR_Ptr_Buffer *inputQueue;
    ///  @brief Worker's thread
    std::thread *workerThread;
    ///  @brief Condition variable, mutex and lockfor the worker
    std::condition_variable *waitCondition;
    std::mutex *d_mutex = new std::mutex();
    std::unique_lock<std::mutex> *lock = new std::unique_lock<std::mutex>(*d_mutex);
    ///  @brief Status of the worker: true if is active
    ///                               false if is sleeping
    bool status = true;
    /// @brief ID of the worker
    int ID;
    /// @brief Number of workers currently active
    int currentWorkers;
    /// @brief Expected TS
    int tsGoal;
    /// @brief Maximum number of workers
    int maxWorkers;
    /// @brief Vector used to store all the final results of the function to be computed
    std::vector<T> accumulator;
    /// @brief Feedback queue (from worker to emitter)
    uSWSR_Ptr_Buffer *feedbackQueue;

    ///  @brief Put this thread in sleep
    ///  @return Void
    void sleep() {
        this->waitCondition->wait(*lock);
    }

    ///  @brief Debug function used to print some useful informations
    ///  @return Void
    void debug(Task<T, U> *t) {
        std::chrono::duration<double> elapsed = t->endingTime - t->startingTime;
        int elapsedINT = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
        int TS = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count() / t->workingThreads;
        int newNWorker = round(float(elapsedINT) / this->tsGoal);
        std::cout << "elapsed " << elapsedINT << " tsGoal " << this->tsGoal << " actual TS " << TS << " current number of workers " << t->workingThreads << " new number of workers: " << newNWorker << std::endl;
    }

    ///  @brief Create a Feedback that the worker will send to the emitter
    ///  @detail I have to check if the new number of workers is 0 or is more thatn
    ///  the maximum number of workers. In the first case i set 1 as the number of
    ///  workers that i want to mantain active, in the second case i set maxWorkers
    ///  as the number of workers that i want to mantain active
    ///  @param int          New number of workers
    ///  @return Feedback
    Feedback createFeedback(int newNWorker) {
        Feedback f;

        if (newNWorker == 0) {
            f.newNumberOfWorkers = 1;
        } else if (newNWorker > this->maxWorkers) {
            f.newNumberOfWorkers = this->maxWorkers;
        } else {
            f.newNumberOfWorkers = newNWorker;
        }
        f.senderID = this->ID;
        return f;
    }

    ///  @brief Wake up a sleeping worker
    ///  @details
    ///  This function pop an item from the input queue.
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
                    return -1;
                } else {
                    t->result = this->function(t->value);
                    t->endingTime = std::chrono::high_resolution_clock::now();
                    accumulator.push_back(t->result);

                    int newNWorker = round(float(std::chrono::duration_cast<std::chrono::milliseconds>(t->endingTime - t->startingTime).count()) / this->tsGoal);
                    //debug(t);

                    if (newNWorker != currentWorkers) {
                        currentWorkers = newNWorker;
                    }
                    Feedback f = createFeedback(newNWorker);
                    this->feedbackQueue->push(&f);

                    return 1;
                }
            }
        }
        return 0;
    }

   public:
    ///  @brief Constructor method of the Worker component
    ///  @param int               Worker's ID
    ///  @param fun               The function to be computed
    ///  @param SWSR_Ptr_Buffer   The queue from which the worker extract the task to be computed
    ///  @param uSWSR_Ptr_Buffer  Feedback Queue
    ///  @param int               activeWorkers
    ///  @param int               tsGoal
    Worker(int ID, std::function<T(U x)> fun, SWSR_Ptr_Buffer *inputQueue, uSWSR_Ptr_Buffer *feedbackQueue, int activeWorkers, int tsGoal) {
        this->function = fun;
        this->inputQueue = inputQueue;
        waitCondition = new std::condition_variable();
        this->status = true;
        this->ID = ID;
        this->feedbackQueue = feedbackQueue;
        this->currentWorkers = activeWorkers;
        this->tsGoal = tsGoal;
        this->maxWorkers = activeWorkers;
    }

    ///  @brief This function starts the Worker and computes the task
    ///  @return Void
    void start() {
        this->workerThread = new std::thread([=] {
            int exit = 0;
            while (compute() != -1) {
                while (!isActive()) {
                    sleep();
                }
                if (exit == 1) break;
            }
            // for (T x : accumulator) {
            //     std::cout << x << std::endl;
            // }
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