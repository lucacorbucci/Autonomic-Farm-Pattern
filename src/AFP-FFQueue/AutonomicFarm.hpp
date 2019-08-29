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
#include <ff/ubuffer.hpp>
#include <ff/mpmc/MPMCqueues.hpp>
#include "../Utils/utimer.hpp"
// clang-format on

using namespace ff;

///  @brief Implementation of the Autonomic Farm Patter
///  @detail Typename T is used for as output type of the function that
///  the worker will compute. Typename U is used as output type of the function
///  that the worker will compute.
template <typename T, typename U>
class AutonomicFarm {
   private:
    std::vector<SWSR_Ptr_Buffer*> inputQueues;
    std::vector<Worker<T, U>*> workerQueue;
    std::vector<Task<T, U>*> inputVector;
    uMPMC_Ptr_Queue* outputQueue = new uMPMC_Ptr_Queue();
    uSWSR_Ptr_Buffer* feedbackQueue;
    uMPMC_Ptr_Queue* feedbackQueueWorker;
    std::function<T(U x)> function;
    int nWorker;
    int tsGoal;
    int time;
    bool collector;
    std::atomic<int>* timeEmitter = new std::atomic<int>(0);
    std::atomic<int>* tsCollector = new std::atomic<int>(0);
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
    AutonomicFarm(int nWorker, std::function<T(U x)> function, int tsGoal, std::vector<Task<T, U>*> inputVector, bool collector, int time, std::string debug) {
        this->nWorker = nWorker;
        this->function = function;
        this->tsGoal = tsGoal;
        this->feedbackQueue = new uSWSR_Ptr_Buffer(1);
        this->feedbackQueueWorker = new uMPMC_Ptr_Queue();
        if (!this->feedbackQueue->init()) {
            abort;
        }
        if (!this->feedbackQueueWorker->init()) {
            abort;
        }
        this->inputVector = inputVector;
        this->outputQueue->init();
        this->collector = collector;
        this->time = time;
        this->debug = debug;
    }

    ///  @brief This function start the creation of the autonomic farm with its components
    /// (emitter, workers, collector)
    ///  @return Void
    void start() {
        // Creation of the communication channels
        for (int i = 0; i < this->nWorker; i++) {
            SWSR_Ptr_Buffer* b = new SWSR_Ptr_Buffer(1);
            if (!b->init()) {
                abort;
            }
            inputQueues.push_back(b);
        }

        // Creation of the workers
        for (int i = 0; i < this->nWorker; i++) {
            this->workerQueue.push_back(new Worker<T, U>{i, this->function, this->inputQueues[i], this->outputQueue, this->feedbackQueueWorker, collector, this->nWorker, this->tsGoal, timeEmitter, debug});
        }

        // Creation of the emitter and the collector
        Emitter<T, U> e{this->inputQueues, workerQueue, nWorker, this->feedbackQueue, this->inputVector, this->feedbackQueueWorker, collector, timeEmitter, time};
        Collector<T, U>* c;
        if (collector) {
            c = new Collector<T, U>(this->outputQueue, this->nWorker, this->tsGoal, this->feedbackQueue, timeEmitter, tsCollector);
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
            delete (inputQueues[i]);
        for (int i = 0; i < nWorker; i++)
            delete (workerQueue[i]->workerThread);
        for (int i = 0; i < nWorker; i++)
            delete (workerQueue[i]);
        delete (feedbackQueue);
        delete (outputQueue);
        delete (feedbackQueueWorker);
        delete (timeEmitter);
        delete (tsCollector);
        if (collector) {
            delete (c);
        }
    }
};
