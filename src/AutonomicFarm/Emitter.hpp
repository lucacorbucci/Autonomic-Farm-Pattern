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
    int nWorker;

   public:
    Emitter(std::vector<boost::lockfree::spsc_queue<Task> *> outputQueue, std::vector<Worker<int, int> *> workerQueue, int nWorker) {
        this->outputQueue = outputQueue;
        this->workerQueue = workerQueue;
        this->nWorker = nWorker;
    }

    long mod(long a, long b) { return (a % b + b) % b; }

    void start(std::vector<Worker<int, int> *> workQueue) {
        std::cout << "Emitter avviato" << std::endl;
        this->emitterThread = std::thread([=] {
            int i = 100;
            auto index = 0;
            while (i > 0) {
                Task task;
                task.workingThreads = this->nWorker;

                index = mod(i, outputQueue.size());
                if (i > 70)
                    task.value = 40;
                else if (i > 30) {
                    task.value = 43;
                    workQueue[0]->stopWorker();
                    workQueue[1]->stopWorker();
                    workQueue[2]->stopWorker();
                    // test
                    task.workingThreads = 2;

                } else {
                    task.value = 45;
                    workQueue[0]->restartWorker();
                    workQueue[1]->restartWorker();
                    workQueue[2]->restartWorker();
                    // test
                    task.workingThreads = 5;
                }

                outputQueue[index]->push(task);

                i--;
            }
            for (int y = 0; y < outputQueue.size(); y++) {
                Task task;
                task.value = -1;
                outputQueue[y]->push(task);
            }
        });
    }

    void join() {
        this->emitterThread.join();
    }
};