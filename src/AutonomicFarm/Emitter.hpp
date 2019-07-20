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

   public:
    Emitter(std::vector<boost::lockfree::spsc_queue<Task> *> outputQueue, std::vector<Worker<int, int> *> workerQueue, int nWorker, boost::lockfree::spsc_queue<Feedback> *feedbackQueue) {
        this->outputQueue = outputQueue;
        this->workerQueue = workerQueue;
        this->nWorker = nWorker;
        this->maxnWorker = nWorker;
        this->feedbackQueue = feedbackQueue;
        bitVector.reserve(nWorker);
        for (int i = 0; i < nWorker; i++) {
            bitVector.push_back(1);
        }
    }

    long mod(long a, long b) { return (a % b + b) % b; }

    void start(std::vector<Worker<int, int> *> workQueue) {
        std::cout << "Emitter avviato" << std::endl;
        this->emitterThread = std::thread([=] {
            int i = 300;
            auto index = 0;
            int currentNumWorker = this->nWorker;
            int prevNumWorker = this->nWorker;
            int dst = 0;
            while (i > 0) {
                Task task;
                task.workingThreads = currentNumWorker;

                index = mod(dst, outputQueue.size());
                if (i > 200)
                    task.value = 43;
                else if (i > 100) {
                    task.value = 40;
                } else {
                    task.value = 44;
                }
                dst++;
                if (workerQueue[index]->isActive() == true) {
                    if (outputQueue[index]->push(task)) {
                        i--;
                        Feedback f;
                        if (this->feedbackQueue->pop(f)) {
                            if (f.newNumberOfWorkers != currentNumWorker) {
                                //std::cout << currentNumWorker << std::endl;
                                prevNumWorker = currentNumWorker;
                                currentNumWorker = f.newNumberOfWorkers;
                                if (currentNumWorker < prevNumWorker) {
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

                                } else if (currentNumWorker > prevNumWorker) {
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
                            }
                        }
                    }
                }
            }
            int sent = 0;
            while (sent < outputQueue.size()) {
                Task task;
                task.value = -1;
                task.workingThreads = this->nWorker;
                if (outputQueue[sent]->push(task)) {
                    sent++;
                }
            }
        });
    }

    void join() {
        this->emitterThread.join();
    }
};