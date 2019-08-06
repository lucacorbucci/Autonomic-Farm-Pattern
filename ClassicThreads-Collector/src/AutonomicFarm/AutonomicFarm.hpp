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
///  the worker will compute. Typename U is input as output type of the function
///  that the worker will compute.
template <typename T, typename U>
class AutonomicFarm {
   private:
    std::vector<SWSR_Ptr_Buffer*> inputQueues;
    uMPMC_Ptr_Queue* outputQueue = new uMPMC_Ptr_Queue();
    std::vector<Worker<T, U>*> workerQueue;
    uSWSR_Ptr_Buffer* feedbackQueue;
    uMPMC_Ptr_Queue* feedbackQueueWorker;

    std::function<T(U x)> function;
    std::vector<Task<T, U>*> inputVector;
    int nWorker;
    int tsGoal;

    ///  @brief Constructor method of the AutonomicFarm Class
    ///  @detail Typename T is used for as output type of the function that
    ///  the worker will compute. Typename U is input as output type of the function
    ///  that the worker will compute.
    ///  @param nWorker     Initial number of workers
    ///  @param function    The function computing the single task in the autonomic farm
    ///  @param tsGoal      Expected service time
    ///  @param inputVector Input Vector with the tasks that has to be computed
   public:
    AutonomicFarm(int nWorker, std::function<T(U x)> function, int tsGoal, std::vector<Task<T, U>*> inputVector) {
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
    }

    ///  @brief This function start the creation of the autonomic farm with its components
    /// (emitter, workers, collector)
    ///  @return Void
    void start() {
        for (int i = 0; i < this->nWorker; i++) {
            SWSR_Ptr_Buffer* b = new SWSR_Ptr_Buffer(1);
            if (!b->init()) {
                abort;
            }
            inputQueues.push_back(b);
        }

        for (int i = 0; i < this->nWorker; i++) {
            this->workerQueue.push_back(new Worker<T, U>{i, this->function, this->inputQueues[i], this->outputQueue, this->feedbackQueueWorker});
        }

        Emitter<T, U> e{this->inputQueues, workerQueue, nWorker, this->feedbackQueue, this->inputVector, this->feedbackQueueWorker};
        Collector<T, U> c{this->outputQueue, this->nWorker, this->tsGoal, this->feedbackQueue};

        {
            utimer t("Tempo Completo");
            c.start();
            for (int i = 0; i < this->nWorker; i++) {
                this->workerQueue[i]->start();
            }

            e.start(workerQueue);

            for (int i = 0; i < this->nWorker; i++) {
                this->workerQueue[i]->join();
            }

            e.join();
            c.join();
        }
    }
};
