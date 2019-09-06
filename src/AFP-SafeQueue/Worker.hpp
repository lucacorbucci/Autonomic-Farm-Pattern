/*
    author: Luca Corbucci
    student number: 516450
*/

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
#ifndef TASK_HPP
#include "../Utils/Task.hpp"
#endif
#ifndef FEEDBACK_HPP
#include "../Utils/Feedback.hpp"
#endif
#ifndef SAFE_QUEUE_HPP
#include "../Utils/safe_queue.h"
#endif
// clang-format on

#define WORKING true;
#define STOPPED false;

///  @brief Implementation of the Worker of the autonomic farm
///  @detail Typename T is used as output type of the function that
///  the worker will compute. Typename U as output type of the function
///  that the worker will compute.
template <class T, class U>
class WorkerSQ {
   private:
    std::function<T(U x)> function;
    SafeQueue<Task<T, U> *> *inputQueue;
    SafeQueue<Task<T, U> *> *outputQueue;
    std::condition_variable *waitCondition;
    std::mutex *d_mutex = new std::mutex();
    std::unique_lock<std::mutex> *lock = new std::unique_lock<std::mutex>(*d_mutex);
    bool status = true;
    int ID;
    SafeQueue<Feedback *> *feedbackQueueWorker;
    int count = 0;
    int x;
    int currentWorkers;
    int maxWorkers;
    int tsGoal;
    std::atomic<int> *timeEmitter;
    std::string debugStr;

    ///  @brief Put this thread in sleep
    ///  @return Void
    void sleep() {
        this->waitCondition->wait(*lock);
    }

    ///  @brief Create a feedback with the number of workers that we want to use
    ///  @returns Void
    void sendFeedback(int newNWorker) {
        Feedback f;

        f.senderID = ID;
        feedbackQueueWorker->safe_push(&f);
    }

    ///  @brief Function used to print some useful information
    ///  @param The task popped from the queue
    void debug(Task<T, U> *t) {
        std::chrono::duration<double> elapsed = t->endingTime - t->startingTime;
        int elapsedINT = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
        int TS = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count() / t->workingThreads;
        int newNWorker = round(float(elapsedINT) / this->tsGoal);
        if (debugStr == "true") {
            std ::cout << "Calcolato " << t->value << " Con " << t->workingThreads << " in " << elapsedINT << " myTS: " << TS << " Ideal TS " << this->tsGoal << " New NWorkers " << newNWorker << " da " << this->ID << std::endl;
        } else if (debugStr == "ts") {
            std::cout << TS << " " << t->workingThreads << std::endl;
        }
    }

    ///  @brief Worker's code
    ///  @details
    ///  This function pop an item from the input queue and then compute the corresponding task
    ///
    ///  @return Returns -1 if it is the last item of the queue and i have to kill
    ///  the worker. In this case i send a Task with value -1 to the collector.
    ///  Returns 0 if it's not the last item. In this case i send the task that i've popped
    ///  from the queue to the collector.
    int compute() {
        Task<T, U> *t;
        t = inputQueue->safe_pop();

        t->startingTime = std::chrono::high_resolution_clock::now();
        if (t->end == -1) {
            outputQueue->safe_push(t);
            return -1;
        } else {
            t->result = this->function(t->value);
            int newNWorker;
            t->endingTime = std::chrono::high_resolution_clock::now();

            debug(t);
            outputQueue->safe_push(t);

            // Send the ack to the emitter
            sendFeedback(newNWorker);

            return 1;
        }

        return 0;
    }

   public:
    std::thread *workerThread;
    ///  @brief Constructor method of the Worker component
    ///  @param ID              Worker's ID
    ///  @param fun             The function to be computed
    ///  @param inputQueue      The queue from which the worker extract the task to be computed
    ///  @param outputQueue     The queue where the worker push the computed task
    ///  @param feedbackQueue   The queue where the worker push the ack for the emitter
    ///  @param activeWorkers   Initial number of workers
    ///  @param tsGoal          Expected service time
    ///  @param timeEmitter     Emitter's service time
    ///  @param debugStr        String used to print some informations during the execution of the farm
    WorkerSQ(int ID, std::function<T(U x)> fun, SafeQueue<Task<T, U> *> *inputQueue, SafeQueue<Task<T, U> *> *outputQueue, SafeQueue<Feedback *> *feedbackQueueWorker, int activeWorkers, int tsGoal, std::atomic<int> *timeEmitter, std::string debugStr) {
        this->function = fun;
        this->inputQueue = inputQueue;
        this->outputQueue = outputQueue;
        waitCondition = new std::condition_variable();
        this->status = true;
        this->ID = ID;
        this->feedbackQueueWorker = feedbackQueueWorker;
        this->x = activeWorkers;
        this->tsGoal = tsGoal;
        this->currentWorkers = activeWorkers;
        this->maxWorkers = activeWorkers;
        this->timeEmitter = timeEmitter;
        this->debugStr = debugStr;
    }

    ///  @brief This function starts the Worker and computes the task
    ///  @return Void
    void start() {
        //std::cout << "avviato worker SQ" << std::endl;
        this->workerThread = new std::thread([=] {
            while (compute() != -1) {
                while (!isActive()) {
                    sleep();
                }
            }
            delete (lock);
            delete (d_mutex);
            delete (waitCondition);
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