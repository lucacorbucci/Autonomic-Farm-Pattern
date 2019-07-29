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

///  @brief Implementation of the emitter of the farm
struct Emitter : ff_monode_t<Task, Task> {
   private:
    ff_loadbalancer *const lb;
    // Number of sent task
    int sent = 0;
    // Number of currently working workers
    int nWorkers;
    // Two vector for the status of the workers
    std::vector<int> activeWorkers;
    std::vector<int> sleepingWorkers;
    // In this vector i store the task that i can't send immediately to a worker
    std::vector<Task *> inputTasks;
    // ID of the last worker that received a task
    int lastWorker = 0;
    // The number of tasks to be computed
    int nTask;
    // The maximum number of workers
    int maxWorkers;
    // In sleeping I count how many threads are sleeping
    int sleeping;
    // In working I count how many threads are working
    int working = 0;

    ///  @brief This function return the index of first active worker
    ///  @return The index of the first active worker
    int
    getFirstActive() {
        for (unsigned int i = this->lastWorker; i < activeWorkers.size(); i++) {
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

    ///  @brief Set the status of a worker as "Working"
    ///  @param int the ID of the thread that is working
    ///  @return Void
    void setWorking(int index) {
        activeWorkers[index] = 0;
    }

    ///  @brief Send a sleep signal to a thread
    ///  @param int the ID of the thread to whom we want to send a sleep signal
    ///  @details
    ///  @return Void
    void setSleeping(int index) {
        ff_send_out_to(GO_OUT, index);
        sleepingWorkers[index] = 1;
        sleeping++;
    }

    ///  @brief Wake up a sleeping worker
    ///  @param int the ID of the thread to whom we want to send a wake up signal
    ///  @return Void
    void wakeUpWorker(int index) {
        sleepingWorkers[index] = 0;
        ff_monode::getlb()->thaw(index, true);
        sleeping--;
    }

    ///  @brief Set a worker's status as "Free"
    ///  @param int the ID of the thread that we want to set free
    ///  @return Void
    void setFree(int index) {
        working--;
        activeWorkers[index] = 1;
    }

    ///  @brief Send a task to a worker
    ///  @param *task the task that we want to compute
    ///  @param int   worker's ID
    ///  @details In this function we change the worker's status, we
    //   send the task to the worker and
    //   we have to increment the variables working and sent.
    ///  @return Void
    void sendTask(Task *task, int worker) {
        task->workingThreads = this->nWorkers;
        setWorking(worker);
        lb->ff_send_out_to(task, worker);
        working++;
        sent++;
    }
    ///  @brief This function check if we have to wake up a worker based on the
    ///  feedback provided by the worker
    ///  @param *Task The feedback provided by the worker
    ///  @return Void
    void checkWakeUp(Task *task) {
        if (task->newWorkingThreads > this->nWorkers) {
            if (task->newWorkingThreads > this->maxWorkers) {
                this->nWorkers = this->maxWorkers;
            } else {
                this->nWorkers = task->newWorkingThreads;
            }

            unsigned int index = 0;
            while (sleeping + this->nWorkers != this->maxWorkers && index < sleepingWorkers.size()) {
                if (sleepingWorkers[index] == 1) {
                    wakeUpWorker(index);
                }
                index++;
            }
        }
    }
    ///  @brief This function check if we have to send a sleep signal to
    ///  one or more worker based on the feedback provided by the worker
    ///  @param *Task The feedback provided by the worker
    ///  @return Void
    void checkSleep(Task *task) {
        if (task->newWorkingThreads < this->nWorkers) {
            if (task->newWorkingThreads == 0) {
                this->nWorkers = 1;
            } else {
                this->nWorkers = task->newWorkingThreads;
            }

            int index = 0;
            while (sleeping + this->nWorkers < this->maxWorkers) {
                if (sleepingWorkers[index] == 0) {
                    setSleeping(index);
                }
                index++;
            }
        }
    }

   public:
    std::vector<int> results;

    ///  @brief Emitter's constructor
    ///  @param *ff_loadbalancer The load balancer of the emitter
    ///  @param int              The initial (and maximum) number of Worker
    ///  @param int              The number of tasks to be computed
    Emitter(ff_loadbalancer *const lb, int nWorkers, int nTask) : lb(lb) {
        this->nWorkers = nWorkers;
        this->nTask = nTask;
        this->maxWorkers = nWorkers;
        this->sleeping = 0;
    }

    int svc_init() {
        /*
            I created two vectors:
            - activeWorkers: here I store the status of the workers that are 
            active
            - sleepingWorkers: here I store the status of the workers that are sleeping
        */
        activeWorkers.reserve(nWorkers);
        sleepingWorkers.reserve(nWorkers);
        for (int i = 0; i < nWorkers; i++) {
            activeWorkers.push_back(1);
            sleepingWorkers.push_back(0);
        }
        this->lastWorker = get_num_outchannels();
        return 0;
    }
    bool finish = false;

    Task *svc(Task *in) {
        int wid = lb->get_channel_id();

        Task *task = reinterpret_cast<Task *>(in);

        /*  
            In this case we receive the task from the ExternalEmitter
            We must send the task to one of the available Workers.
         */
        if ((int)wid == -1) {
            int worker = getFirstActive();
            if (worker == -1) {
                inputTasks.push_back(task);
                return GO_ON;
            } else {
                sendTask(task, worker);
            }
        }

        // In this case we received the feedback from one of the workers
        if ((size_t)wid < get_num_outchannels()) {
            setFree(wid);

            // wake up one or more worker based on the information of the feedback
            checkWakeUp(task);

            // send a sleep to one or more worker based on the information of the feedback
            checkSleep(task);

            /*
                I have to check if there are some tasks in the temporary queue.
                If there are some tasks i send one (or more) task to a worker.
            */
            if (inputTasks.size() > 0) {
                /*
                    This while is necessary because when i decrease the number of 
                    workers and then I increase this number, i have to send 
                    a task to a worker even if it didn't send a feedback because it 
                    was sleeping.
                */
                while (this->working < this->nWorkers) {
                    int index = getFirstActive();

                    if (index >= 0) {
                        Task *inTask = reinterpret_cast<Task *>(inputTasks.front());
                        inTask->workingThreads = this->nWorkers;
                        sendTask(inTask, index);
                        inputTasks.erase(inputTasks.begin());
                    }
                }
            }
        }

        // I have to check if i send all the tasks to the workers.
        if (sent == nTask) {
            /*
                In this case i send a wake up signal to all the workers
                and then i send an EOS to all the workers.
            */
            for (int i = 0; i < this->maxWorkers; i++) {
                wakeUpWorker(i);
                //std::cout << "wake up " << i << std::endl;
            }
            lb->broadcast_task(EOS);
            return EOS;
        } else {
            return GO_ON;
        }
    }
};