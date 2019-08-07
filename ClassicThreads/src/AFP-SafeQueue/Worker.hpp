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
///  @detail Typename T is used for as output type of the function that
///  the worker will compute. Typename U is input as output type of the function
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

    ///  @brief Put this thread in sleep
    ///  @return Void
    void sleep() {
        this->waitCondition->wait(*lock);
    }

    ///  @brief Create a feedback with the number of workers that we want to use
    ///  @return The feedback to be sent
    Feedback createFeedback() {
        Feedback f;
        f.senderID = ID;
        return f;
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
        Task<T, U> *t;

        t = inputQueue->safe_pop();
        t->startingTime = std::chrono::high_resolution_clock::now();
        if (t->end == -1) {
            outputQueue->safe_push(t);
            return -1;
        } else {
            t->result = this->function(t->value);
            t->endingTime = std::chrono::high_resolution_clock::now();

            outputQueue->safe_push(t);
            Feedback f = createFeedback();
            feedbackQueueWorker->safe_push(&f);
            return 1;
        }

        return 0;
    }

   public:
    std::thread *workerThread;
    ///  @brief Constructor method of the Worker component
    ///  @param fun          The function to be computed
    ///  @param inputQueue   The queue from which the worker extract the task to be computed
    ///  @param outputQueue  The queue where the worker push the computed task
    WorkerSQ(int ID, std::function<T(U x)> fun, SafeQueue<Task<T, U> *> *inputQueue, SafeQueue<Task<T, U> *> *outputQueue, SafeQueue<Feedback *> *feedbackQueueWorker, bool collector, int activeWorkers, int tsGoal) {
        this->function = fun;
        this->inputQueue = inputQueue;
        this->outputQueue = outputQueue;
        waitCondition = new std::condition_variable();
        this->status = true;
        this->ID = ID;
        this->feedbackQueueWorker = feedbackQueueWorker;
    }

    ///  @brief This function starts the Worker and computes the task
    ///  @return Void
    void start() {
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