// clang-format off
#include <unistd.h>
#include <iostream>
#include <utility>
#include <vector>
#include <boost/lockfree/spsc_queue.hpp>
// clang-format on

template <class T>
class Emitter {
   private:
    std::vector<boost::lockfree::spsc_queue<Task> *> outputQueue;
    std::thread emitterThread;
    std::vector<Worker<int, int> *> workerQueue;
    boost::lockfree::spsc_queue<Feedback> *feedbackQueue;
    std::vector<int> bitVector;
    std::vector<Task> inputVector;
    int nWorker;
    int maxnWorker;

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

    void sendSleepSignal(int currentNumWorker, int prevNumWorker) {
        int toSleep = prevNumWorker - currentNumWorker;
        int j = 0;
        while (j < toSleep) {
            int index = getFirstActive();

            if (index != -1) {
                bitVector[index] = 0;
                if (workerQueue[index]->stopWorker() == false) {
                    ;
                }
            }
            j++;
        }
    }

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

   public:
    Emitter(std::vector<boost::lockfree::spsc_queue<Task> *> outputQueue, std::vector<Worker<int, int> *> workerQueue, int nWorker, boost::lockfree::spsc_queue<Feedback> *feedbackQueue, std::vector<Task> inputVector) {
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

    long mod(long a, long b) { return (a % b + b) % b; }

    void updateWorkers(int prevNumWorker, int currentNumWorker) {
        currentNumWorker < prevNumWorker
            ? sendSleepSignal(currentNumWorker, prevNumWorker)
            : sendWakeUpSignal(currentNumWorker, prevNumWorker);
    }

    void threadBody() {
        int i = 0, index = 0, currentNumWorker = this->nWorker, prevNumWorker = this->nWorker, dst = 0;
        while (i < inputVector.size()) {
            Task task = inputVector.at(i);
            task.workingThreads = currentNumWorker;
            index = mod(dst, outputQueue.size());
            dst++;

            if (workerQueue[index]->isActive()) {
                if (outputQueue[index]->push(task)) {
                    i++;
                    Feedback f;
                    if (this->feedbackQueue->pop(f)) {
                        if (f.newNumberOfWorkers != currentNumWorker) {
                            prevNumWorker = currentNumWorker;
                            currentNumWorker = f.newNumberOfWorkers;
                            updateWorkers(prevNumWorker, currentNumWorker);
                        }
                    }
                }
            }
        }

        // I have to send the final task with value -1 to stop the workers
        int sent = 0;
        while (sent < outputQueue.size()) {
            Task task;
            task.value = -1;
            task.workingThreads = this->nWorker;
            if (outputQueue[sent]->push(task)) {
                sent++;
            }
        }
    }

    void start(std::vector<Worker<int, int> *> workQueue) {
        std::cout << "Emitter avviato" << std::endl;
        this->emitterThread = std::thread([=] {
            threadBody();
        });
    }

    void join() {
        this->emitterThread.join();
    }
};