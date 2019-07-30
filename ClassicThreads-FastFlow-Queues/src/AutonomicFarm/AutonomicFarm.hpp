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
// clang-format on

using namespace ff;

///  @brief Implementation of the Autonomic Farm Pattern

template <class T>
class AutonomicFarm {
   private:
    //std::vector<boost::lockfree::spsc_queue<Task>*> inputQueues;
    std::vector<SWSR_Ptr_Buffer*> inputQueues;
    uMPMC_Ptr_Queue* outputQueue = new uMPMC_Ptr_Queue();
    std::vector<Worker<int, int>*> workerQueue;
    uSWSR_Ptr_Buffer* feedbackQueue;
    //boost::lockfree::spsc_queue<Feedback>* feedbackQueue;
    std::function<int(int x)> function;
    std::vector<Task*> inputVector;
    int nWorker;
    int tsGoal;

    ///  @brief Constructor method of the AutonomicFarm Class
    ///
    ///  @param nWorker     Initial number of workers
    ///  @param function    The function computing the single task in the autonomic farm
    ///  @param tsGoal      Expected service time
    ///  @param inputVector Input Vector with the tasks that has to be computed
    ///
   public:
    AutonomicFarm(int nWorker, std::function<int(int x)> function, int tsGoal, std::vector<Task*> inputVector) {
        this->nWorker = nWorker;
        this->function = function;
        this->tsGoal = tsGoal;
        this->feedbackQueue = new uSWSR_Ptr_Buffer(1);
        if (!this->feedbackQueue->init()) {
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
            this->workerQueue.push_back(new Worker<int, int>{i, this->function, this->inputQueues[i], this->outputQueue});
        }

        Emitter<int> e{this->inputQueues, workerQueue, nWorker, this->feedbackQueue, this->inputVector};

        for (int i = 0; i < this->nWorker; i++) {
            this->workerQueue[i]->start();
        }

        e.start(workerQueue);

        Collector<int> c{this->outputQueue, this->nWorker, this->tsGoal, this->feedbackQueue};
        c.start();

        for (int i = 0; i < this->nWorker; i++) {
            this->workerQueue[i]->join();
        }

        e.join();
        c.join();
    }
};
