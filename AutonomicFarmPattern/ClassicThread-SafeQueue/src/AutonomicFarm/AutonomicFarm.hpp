
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
// clang-format on

///  @brief Implementation of the Autonomic Farm Patter
///  @detail Typename T is used for as output type of the function that
///  the worker will compute. Typename U is input as output type of the function
///  that the worker will compute.
template <typename T, typename U>
class AutonomicFarm {
   private:
    std::vector<SafeQueue<Task<T, U>*>*>* inputQueues = new std::vector<SafeQueue<Task<T, U>*>*>();
    SafeQueue<Task<T, U>*>* outputQueue = new SafeQueue<Task<T, U>*>();
    std::vector<Worker<T, U>*> workerQueue;

    SafeQueue<Feedback*>* feedbackQueue;
    SafeQueue<Feedback*>* feedbackQueueWorker;

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
        this->feedbackQueue = new SafeQueue<Feedback*>();
        this->feedbackQueueWorker = new SafeQueue<Feedback*>();

        this->inputVector = inputVector;
    }

    ///  @brief This function start the creation of the autonomic farm with its components
    /// (emitter, workers, collector)
    ///  @return Void
    void start() {
        for (int i = 0; i < this->nWorker; i++) {
            SafeQueue<Task<T, U>*>* b = new SafeQueue<Task<T, U>*>();

            inputQueues->push_back(b);
        }

        for (int i = 0; i < this->nWorker; i++) {
            //std::cout << "Autonomic " << &(*this->inputQueues)[i] << std::endl;
            this->workerQueue.push_back(new Worker<T, U>{i, this->function, &(*this->inputQueues)[i], this->outputQueue, this->feedbackQueueWorker});
        }

        Emitter<T, U> e{this->inputQueues, workerQueue, nWorker, this->feedbackQueue, this->inputVector, this->feedbackQueueWorker};

        Collector<T, U> c{this->outputQueue, this->nWorker, this->tsGoal, this->feedbackQueue};

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
};
