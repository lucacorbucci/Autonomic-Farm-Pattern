// clang-format off
#include <unistd.h>
#include <iostream>
#include <utility>
#include <vector>
#include <ff/ff.hpp>
#include <ff/farm.hpp>
#include <cstdio>
#include <math.h>
// clang-format on

using namespace ff;

struct Emitter : ff_monode_t<Task, Task> {
   private:
    ff_loadbalancer *const lb;
    int sent = 0;
    int nWorkers;
    std::vector<int> activeWorkers;
    std::vector<int> sleepingWorkers;

    std::vector<Task *> inputTasks;
    int lastWorker = 0;
    int lastWorkerSleep = 0;
    int nTask;
    int maxWorkers;
    // qua conto quanti thread dormono
    int sleeping;
    int working = 0;

    ///  @brief This function return the index of first active worker
    ///  @return The index of the first active worker
    int getFirstActive() {
        for (int i = this->lastWorker; i < activeWorkers.size(); i++) {
            if (activeWorkers[i] == 1 && sleepingWorkers[i] == 0) {
                this->lastWorker = i;
                return i;
            }
        }

        for (int i = 0; i < this->lastWorker; i++) {
            if (activeWorkers[i] == 1 && sleepingWorkers[i] == 0) {
                this->lastWorker = i;
                return i;
            }
        }
        return -1;
    }

    int getFirstSleeping() {
        for (int i = this->lastWorkerSleep; i < sleepingWorkers.size(); i++) {
            if (sleepingWorkers[i] == 1) {
                this->lastWorkerSleep = i;
                return i;
            }
        }

        for (int i = 0; i < this->lastWorkerSleep; i++) {
            if (sleepingWorkers[i] == 1) {
                this->lastWorkerSleep = i;
                return i;
            }
        }
        return -1;
    }

    void setWorking(int index) {
        activeWorkers[index] = 0;
    }

    void setSleeping(int index) {
        ff_send_out_to(GO_OUT, index);
        sleepingWorkers[index] = 1;
    }

    void wakeUpWorker(int index) {
        sleepingWorkers[index] = 0;
        ff_monode::getlb()->thaw(index, true);
    }

    void setFree(int index) {
        activeWorkers[index] = 1;
    }

    void sendTask(Task *task, int worker) {
        task->workingThreads = this->nWorkers;
        setWorking(worker);
        lb->ff_send_out_to(task, worker);
        sent++;
    }

   public:
    std::vector<int> results;

    Emitter(ff_loadbalancer *const lb, int nWorkers, int nTask) : lb(lb) {
        this->nWorkers = nWorkers;
        this->nTask = nTask;
        this->maxWorkers = nWorkers;
        this->sleeping = 0;
    }

    int svc_init() {
        activeWorkers.reserve(nWorkers);
        sleepingWorkers.reserve(nWorkers);
        for (int i = 0; i < nWorkers; i++) {
            activeWorkers.push_back(1);
            sleepingWorkers.push_back(0);
        }
        this->lastWorker = get_num_outchannels();
        return 0;
    }

    Task *svc(Task *in) {
        int wid = lb->get_channel_id();
        Task *task = reinterpret_cast<Task *>(in);

        // In this case we receive the task from the ExternalEmitter
        // We must send the task to one of the available Workers.
        if ((size_t)wid == -1) {
            int worker = getFirstActive();
            if (worker == -1) {
                inputTasks.push_back(task);
                return GO_ON;
            } else {
                sendTask(task, worker);
            }
        }
        // In this case we received the feedback from one of the workers
        else if ((size_t)wid < get_num_outchannels()) {
            // inTask is the task that we estract from the temporary
            // vector where we store the tasks that we receive from
            // the external emitter that we can't send immediately to one worker.s
            setFree(wid);
            int toSleep = 0;
            int wake = 0;

            // RISVEGLIARE
            if (task->newWorkingThreads > this->nWorkers) {
                int toWakeUp;
                if (task->newWorkingThreads > this->maxWorkers) {
                    toWakeUp = this->maxWorkers - this->nWorkers;
                    this->nWorkers = this->maxWorkers;
                } else {
                    toWakeUp = task->newWorkingThreads - this->nWorkers;
                    this->nWorkers = task->newWorkingThreads;
                }

                int index = 0;
                while (sleeping + this->nWorkers != this->maxWorkers && index < sleepingWorkers.size()) {
                    if (sleepingWorkers[index] == 1) {
                        std::cout << "Wake up" << std::endl;
                        wakeUpWorker(index);
                        toWakeUp--;
                        sleeping--;
                    }
                    index++;
                }

            }
            // ADDORMENTARE -> ok funziona
            else if (task->newWorkingThreads < this->nWorkers) {
                int toSleep;
                if (task->newWorkingThreads == 0) {
                    this->nWorkers = 1;
                } else {
                    this->nWorkers = task->newWorkingThreads;
                }

                int index = 0;
                while (sleeping + this->nWorkers != this->maxWorkers) {
                    if (sleepingWorkers[index] == 0) {
                        setSleeping(index);
                        toSleep--;
                        sleeping++;
                    }
                    index++;
                }
            }

            int index = getFirstActive();
            std::cout << "Index " << index << std::endl;
            if (inputTasks.size() > 0 && index >= 0) {
                Task *inTask = reinterpret_cast<Task *>(inputTasks.front());
                inTask->workingThreads = this->nWorkers;
                sendTask(inTask, index);
                inputTasks.erase(inputTasks.begin());
            }
        }
        if (sent == nTask) {
            return EOS;

        } else {
            return GO_ON;
        }
    }
};