// clang-format off
#include <unistd.h>
#include <iostream>
#include <utility>
#include <vector>
#include <ff/ubuffer.hpp>

// clang-format on
using namespace ff;

///  @brief Implementation of the Emitter of the autonomic farm
template <class T>
class Emitter {
   private:
    std::vector<SWSR_Ptr_Buffer *> outputQueue;
    std::thread emitterThread;
    std::vector<Worker<int, int> *> workerQueue;
    //boost::lockfree::spsc_queue<Feedback> *feedbackQueue;
    uSWSR_Ptr_Buffer *feedbackQueue;
    std::vector<int> bitVector;
    std::vector<Task *> inputVector;
    int nWorker;
    int maxnWorker;

    ///  @brief This function return the index of first active worker
    ///  @return The index of the first active worker
    int getFirstActive() {
        int index = 0;
        int f = 0;
        for (int x : bitVector) {
            if (x == 1) {
                return index;
            } else
                index++;
        }
        return -1;
    }

    ///  @brief This function return the index of first sleeping worker
    ///  @return The index of the first sleeping worker
    int getFirstSleeping() {
        int index = 0;
        for (auto x : bitVector) {
            if (x == 0)
                return index;
            else
                index++;
        }
        return -1;
    }

    ///  @brief This function send a sleep signal to a worker
    ///  @return Void
    void sendSleepSignal(int currentNumWorker, int prevNumWorker) {
        int toSleep = prevNumWorker - currentNumWorker;
        int j = 0;
        while (j < toSleep) {
            int index = getFirstActive();

            if (index != -1) {
                bitVector[index] = 0;
                workerQueue[index]->stopWorker();
            }
            j++;
        }
    }

    ///  @brief Wake up a sleeping worker
    ///  @details
    ///  This function compute the number of workers that it has to wake up
    ///  and then send a restart signal to the thread.
    ///  @param
    ///  @param
    ///  @return Void
    void sendWakeUpSignal(int currentNumWorker, int prevNumWorker) {
        int toWakeUp = currentNumWorker - prevNumWorker;
        int j = 0;
        while (j < toWakeUp) {
            int index = getFirstSleeping();
            if (index != -1) {
                bitVector[index] = 1;
                if (workerQueue[index]->restartWorker() == true) {
                    ;
                }
            }
            j++;
        }
    }

    ///  @brief Compute the modulo between and b
    ///  @return The modulo between and b
    long mod(long a, long b) { return (a % b + b) % b; }

    ///  @brief Send a sleep or a wake up signal to one of the worker
    ///  @return Void
    void updateWorkers(int prevNumWorker, int currentNumWorker) {
        currentNumWorker < prevNumWorker
            ? sendSleepSignal(currentNumWorker, prevNumWorker)
            : sendWakeUpSignal(currentNumWorker, prevNumWorker);
    }

    ///  @brief code of the emitter of the autonomic farm
    ///  @details
    ///  This function extracts a task from the input vector and push this task
    ///  to the input spsc queue of an active worker.
    ///  This function also extracts  a feedback from the spsc queue connected with
    ///  the collector  and based on this feedback send a sleep or a wake up signal
    ///  to one of the workers.
    ///  @return Void
    void threadBody() {
        int i = 0, index = 0, currentNumWorker = this->nWorker, prevNumWorker = this->nWorker, dst = 0;
        while (i < inputVector.size()) {
            Task *task = inputVector.at(i);
            task->workingThreads = currentNumWorker;

            index = mod(dst, outputQueue.size());
            dst++;

            if (workerQueue[index]->isActive()) {
                void *taskVoid = task;

                Task *t = (Task *)taskVoid;

                if (outputQueue[index]->push(task)) {
                    i++;

                    void *tmpF;
                    bool res = this->feedbackQueue->pop(&tmpF);
                    //std::cout  <<  res << std::endl;
                    if (res) {
                        Feedback *f = reinterpret_cast<Feedback *>(tmpF);

                        if (f->newNumberOfWorkers != currentNumWorker) {
                            prevNumWorker = currentNumWorker;
                            currentNumWorker = f->newNumberOfWorkers;
                            updateWorkers(prevNumWorker, currentNumWorker);
                        }
                    }
                }
            }
        }

        // I have to send the final task with value -1 to stop the workers
        int sent = 0;
        int d = getFirstSleeping();
        while (d != -1) {
            workerQueue[d]->restartWorker();
            bitVector[d] = 1;
            d = getFirstSleeping();
        }

        while (sent < outputQueue.size()) {
            Task *task = new Task();
            task->value = -1;
            if (outputQueue[sent]->push(task)) {
                sent++;
            }
        }
    }

   public:
    ///  @brief Constructor method of the Emitter component
    ///  @param *outputQueue   The queue where the emitter send a new task
    ///  @param workerQueue    The workers's queue
    ///  @param nWorker        The initial number of active workers
    ///  @param FeedbackQueue  The feedback queue used to send back a feedback to the emitter
    ///  @param inputVector    Input Vector with the tasks that has to be computed
    Emitter(std::vector<SWSR_Ptr_Buffer *> outputQueue, std::vector<Worker<int, int> *> workerQueue, int nWorker, uSWSR_Ptr_Buffer *feedbackQueue, std::vector<Task *> inputVector) {
        this->outputQueue = outputQueue;
        this->workerQueue = workerQueue;
        this->nWorker = nWorker;
        this->maxnWorker = nWorker;
        this->feedbackQueue = feedbackQueue;
        this->inputVector = inputVector;
        bitVector.reserve(nWorker);
        for (int i = 0; i < nWorker; i++) {
            bitVector.push_back(1);
        }
    }

    ///  @brief Start the thread of the emitter componentend.
    ///  @return Void
    void start(std::vector<Worker<int, int> *> workQueue) {
        // std::cout << "Emitter avviato" << std::endl;
        this->emitterThread = std::thread([=] {
            threadBody();
        });
    }

    ///  @brief This function is used to join the emitter thread
    ///  @return Void
    void join() {
        this->emitterThread.join();
    }
};