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
    std::atomic<int>* timeEmitter = new std::atomic<int>(0);
    std::atomic<int>* tsCollector = new std::atomic<int>(0);
    std::string debug;
    int inputSize;
    U n1, n2, n3;

    ///  @brief Creates the input vector with all the tasks that i have to compute
    void fillVector() {
        std::vector<Task<T, U>*> inputVector;
        inputVector.reserve(this->inputSize);
        for (int i = 0; i < this->inputSize; i++) {
            Task<T, U>* task = new Task<T, U>();
            if (i > 2 * (this->inputSize / 3))
                task->value = this->n3;
            else if (i > (this->inputSize / 3)) {
                task->value = this->n2;
            } else {
                task->value = this->n1;
            }
            inputVector.push_back(task);
        }
        this->inputVector = inputVector;
    }

    ///  @brief This function optionally prints the results of the computation
    ///  and then delete the task
    void printAndDelete(std::vector<Task<T, U>*> accumulator) {
        if (debug == "results") {
            for (auto item : accumulator) {
                std::cout << item->result << std::endl;
            }
        }
        for (auto item : accumulator) {
            delete (item);
        }
    }

   public:
    ///  @brief Constructor method of the AutonomicFarm Class
    ///  @detail Typename T is used as output type of the function that
    ///  the worker will compute. Typename U is used as output type of the function
    ///  that the worker will compute.
    ///  @param nWorker     Initial number of workers
    ///  @param function    The function that the worker will compute
    ///  @param tsGoal      Expected service time
    ///  @param inputVector Input Vector with the tasks that has to be computed
    ///  @param time        This is time that we have to wait to change the number of workers of the farm
    ///  @param debug       This is used to print some informations during the execution of the farm
    AutonomicFarm(int nWorker, std::function<T(U x)> function, int tsGoal, int inputSize, U n1, U n2, U n3, int time, std::string debug) {
        this->nWorker = nWorker;
        this->function = function;
        this->tsGoal = tsGoal;
        this->feedbackQueue = new uSWSR_Ptr_Buffer(1);
        this->feedbackQueueWorker = new uMPMC_Ptr_Queue();
        if (!this->feedbackQueue->init()) {
            abort();
        }
        if (!this->feedbackQueueWorker->init()) {
            abort();
        }
        this->outputQueue->init();
        this->time = time;
        this->debug = debug;
        this->inputSize = inputSize;
        this->n1 = n1;
        this->n2 = n2;
        this->n3 = n3;
        fillVector();
    }

    ///  @brief This function start the creation of the autonomic farm with its components
    /// (emitter, workers, collector)
    ///  @return Void
    void start() {
        {
            utimer t("Time");
            // Creation of the communication channels
            for (int i = 0; i < this->nWorker; i++) {
                SWSR_Ptr_Buffer* b = new SWSR_Ptr_Buffer(1);
                if (!b->init()) {
                    abort();
                }
                inputQueues.push_back(b);
            }

            // Creation of the workers
            for (int i = 0; i < this->nWorker; i++) {
                this->workerQueue.push_back(new Worker<T, U>{i, this->function, this->inputQueues[i], this->outputQueue, this->feedbackQueueWorker, this->nWorker, this->tsGoal, timeEmitter, debug});
            }

            // Creation of the emitter and the collector
            Emitter<T, U> e{this->inputQueues, workerQueue, nWorker, this->feedbackQueue, this->inputVector, this->feedbackQueueWorker, timeEmitter, time};
            Collector<T, U>* c = new Collector<T, U>(this->outputQueue, this->nWorker, this->tsGoal, this->feedbackQueue, timeEmitter, tsCollector);

            c->start();
            for (int i = 0; i < this->nWorker; i++) {
                this->workerQueue[i]->start();
            }

            e.start();

            for (int i = 0; i < this->nWorker; i++) {
                this->workerQueue[i]->join();
            }

            e.join();
            printAndDelete(c->accumulator);

            // Free the memory
            c->join();
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
            delete (c);
        }
    }
};
