// clang-format off
#include <unistd.h>
#include <iostream>
#include <utility>
#include <vector>
#include "Worker.hpp"
#include "Emitter.hpp"
#include <mutex>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <ff/ubuffer.hpp>
#include <ff/mpmc/MPMCqueues.hpp>
// clang-format on

using namespace ff;

///  @brief Implementation of the Autonomic Farm Patter
///  @detail Typename T is used for as output type of the function that
///  the worker will compute. Typename U is input as output type of the function
///  that the worker will compute.
template <typename T, typename U>
class AutonomicFarm {
   private:
    ///  @brief Vector of queues used to send tasks from the emitter to the workers
    std::vector<SWSR_Ptr_Buffer*> inputQueues;
    ///  @brief Vector of workers
    std::vector<Worker<T, U>*> workerQueue;
    ///  @brief Feedback queue used to send feedback from worker to sender
    uSWSR_Ptr_Buffer* feedbackQueue;
    ///  @brief The function to be computed by the worker
    std::function<T(U x)> function;
    ///  @brief Vector with input data
    std::vector<Task<T, U>*> inputVector;
    ///  @brief Number of workers
    int nWorker;
    ///  @brief Expected Ts
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
        if (!this->feedbackQueue->init()) {
            abort;
        }
        this->inputVector = inputVector;
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

        // Fill the worker queue
        for (int i = 0; i < this->nWorker; i++) {
            this->workerQueue.push_back(new Worker<T, U>{i, this->function, this->inputQueues[i], this->feedbackQueue, this->nWorker, this->tsGoal});
        }

        // Create the emitter
        Emitter<T, U> e{this->inputQueues, workerQueue, nWorker, this->feedbackQueue, this->inputVector};

        // Start the workers
        for (int i = 0; i < this->nWorker; i++) {
            this->workerQueue[i]->start();
        }

        // start the emitter
        e.start(workerQueue);

        for (int i = 0; i < this->nWorker; i++) {
            this->workerQueue[i]->join();
        }

        e.join();
    }
};
