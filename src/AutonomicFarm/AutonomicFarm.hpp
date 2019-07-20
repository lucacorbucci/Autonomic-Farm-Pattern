// clang-format off
#include <unistd.h>
#include <iostream>
#include <utility>
#include <vector>
#include <boost/lockfree/spsc_queue.hpp>
#include <boost/lockfree/queue.hpp>
#include "Worker.hpp"
#include "Emitter.hpp"
#include "Collector.hpp"
#include <mutex>
#include <chrono>
#include <condition_variable>
#include <mutex>
// clang-format on
// @tparam T   Any float-point type such as float, double or long double

#define MAX_SIZE

///  @brief Implementation of the Autonomic Farm Pattern

template <class T>
class AutonomicFarm {
   private:
    std::vector<boost::lockfree::spsc_queue<Task>*> inputQueues;
    boost::lockfree::queue<Task>* outputQueue = new boost::lockfree::queue<Task>();
    std::vector<Worker<int, int>*> workerQueue;
    boost::lockfree::spsc_queue<Feedback>* feedbackQueue;
    std::function<int(int x)> function;
    std::vector<Task> inputVector;
    int nWorker;
    int tsGoal;

    ///  @brief Constructor method of the AutonomicFarm Class
    ///
    ///  @param nWorker     Initial number of workers
    ///  @param function    The function computing the single task in the autonomic farm
    ///  @param tsGoal      Expected service time (TS)
    ///  @param inputVector Input Vector with the tasks that has to be computed
    ///
   public:
    AutonomicFarm(int nWorker, std::function<int(int x)> function, int tsGoal, std::vector<Task> inputVector) {
        this->nWorker = nWorker;
        this->function = function;
        this->tsGoal = tsGoal;
        this->feedbackQueue = new boost::lockfree::spsc_queue<Feedback>{1024 * 1024};
        this->inputVector = inputVector;
    }

    ///  @brief This function start the creation of the autonomic farm with its components
    /// (emitter, workers, collector)
    ///  @return Void
    void start() {
        for (int i = 0; i < this->nWorker; i++) {
            inputQueues.push_back(new boost::lockfree::spsc_queue<Task>{1});
        }

        for (int i = 0; i < this->nWorker; i++) {
            this->workerQueue.push_back(new Worker<int, int>{this->function, this->inputQueues[i], this->outputQueue});
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
