/*
    author: Luca Corbucci
    student number: 516450
*/

// clang-format off
#include <unistd.h>
#include <iostream>
#include <utility>
#include <vector>
#include "Worker.hpp"
#include "Emitter.hpp"
#include "Collector.hpp"
#include <mutex>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include "../Utils/utimer.hpp"
// clang-format on

///  @brief Implementation of the Autonomic Farm Patter
///  @detail Typename T is used for as output type of the function that
///  the worker will compute. Typename U is used as output type of the function
///  that the worker will compute.
template <typename T, typename U>
class AutonomicFarmSQ {
   private:
    std::vector<SafeQueue<Task<T, U>*>*>* inputQueues = new std::vector<SafeQueue<Task<T, U>*>*>();
    std::vector<WorkerSQ<T, U>*> workerQueue;
    std::vector<Task<T, U>*> inputVector;
    SafeQueue<Task<T, U>*>* outputQueue = new SafeQueue<Task<T, U>*>();
    SafeQueue<Feedback*>* feedbackQueue;
    SafeQueue<Feedback*>* feedbackQueueWorker;
    std::function<T(U x)> function;
    int nWorker;
    int time;
    int tsGoal;
    bool collector;
    std::atomic<int>* timeEmitter = new std::atomic<int>(0);
    std::atomic<int>* timeCollector = new std::atomic<int>(0);
    std::string debug;

   public:
    ///  @brief Constructor method of the AutonomicFarm Class
    ///  @detail Typename T is used as output type of the function that
    ///  the worker will compute. Typename U is used as output type of the function
    ///  that the worker will compute.
    ///  @param nWorker     Initial number of workers
    ///  @param function    The function that the worker will compute
    ///  @param tsGoal      Expected service time
    ///  @param inputVector Input Vector with the tasks that has to be computed
    ///  @param collector   True if we want to use the collector, false otherwise
    ///  @param time        This is time that we have to wait to change the number of workers of the farm
    ///  @param debug       This is used to print some informations during the execution of the farm
    AutonomicFarmSQ(int nWorker, std::function<T(U x)> function, int tsGoal, std::vector<Task<T, U>*> inputVector, bool collector, int time, std::string debug) {
        this->nWorker = nWorker;
        this->function = function;
        this->tsGoal = tsGoal;
        this->feedbackQueue = new SafeQueue<Feedback*>();
        this->feedbackQueueWorker = new SafeQueue<Feedback*>();
        this->debug = debug;

        this->inputVector = inputVector;
        this->collector = collector;
        this->time = time;
    }

    ///  @brief This function start the creation of the autonomic farm with its components
    /// (emitter, workers, collector)
    ///  @return Void
    void start() {
        // Creation of the communication channels
        for (int i = 0; i < this->nWorker; i++) {
            SafeQueue<Task<T, U>*>* b = new SafeQueue<Task<T, U>*>();

            inputQueues->push_back(b);
        }

        // Creation of the workers
        for (int i = 0; i < this->nWorker; i++) {
            this->workerQueue.push_back(new WorkerSQ<T, U>{i, this->function, inputQueues->at(i), this->outputQueue, this->feedbackQueueWorker, this->collector, this->nWorker, this->tsGoal, this->timeEmitter, debug});
        }

        // Creation of the emitter and the collector
        EmitterSQ<T, U> e{this->inputQueues, workerQueue, nWorker, this->feedbackQueue, this->inputVector, this->feedbackQueueWorker, this->collector, this->timeEmitter, time};

        CollectorSQ<T, U>* c;

        if (collector) {
            c = new CollectorSQ<T, U>(this->outputQueue, this->nWorker, this->tsGoal, this->feedbackQueue, timeEmitter, timeCollector);
        }

        {
            utimer t("Tempo");
            if (collector) {
                c->start();
            }

            for (int i = 0; i < this->nWorker; i++) {
                this->workerQueue[i]->start();
            }
            e.start();

            for (int i = 0; i < this->nWorker; i++) {
                this->workerQueue[i]->join();
            }

            e.join();
            if (collector) {
                c->join();
            }
        }

        for (int i = 0; i < nWorker; i++)
            delete (inputQueues->at(i));
        for (int i = 0; i < nWorker; i++)
            delete (workerQueue[i]->workerThread);
        for (int i = 0; i < nWorker; i++)
            delete (workerQueue[i]);
        delete (feedbackQueue);
        delete (inputQueues);
        delete (outputQueue);
        delete (feedbackQueueWorker);
        delete (timeEmitter);
        delete (timeCollector);
        if (collector) {
            delete (c);
        }
    }
};