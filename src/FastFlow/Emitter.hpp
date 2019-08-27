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
template <typename T, typename U>
struct EmitterFF : ff_monode_t<Task<T, U>, Task<T, U>> {
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
    std::vector<Task<T, U> *> inputTasks;
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

    int x;
    int count = 0;

    int received = 0;

    bool eosReceived = false;

    std::chrono::high_resolution_clock::time_point lastUpdate;
    int stopTime;
    bool first = true;

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
        this->ff_send_out_to(this->GO_OUT, index);
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
    void sendTask(Task<T, U> *task, int worker) {
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
    void checkWakeUp(Task<T, U> *task) {
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
                    lastUpdate = std::chrono::high_resolution_clock::now();
                    first = false;
                }
                index++;
            }
            count = 0;
        }
    }
    ///  @brief This function check if we have to send a sleep signal to
    ///  one or more worker based on the feedback provided by the worker
    ///  @param *Task The feedback provided by the worker
    ///  @return Void
    void checkSleep(Task<T, U> *task) {
        if (task->newWorkingThreads < this->nWorkers) {
            if (task->newWorkingThreads == 0) {
                this->nWorkers = 1;
            } else {
                this->nWorkers = task->newWorkingThreads;
            }

            unsigned int index = 0;
            while (sleeping + this->nWorkers < this->maxWorkers && index < sleepingWorkers.size()) {
                if (sleepingWorkers[index] == 0 && activeWorkers[index] == 1) {
                    setSleeping(index);
                    lastUpdate = std::chrono::high_resolution_clock::now();
                    first = false;
                }
                index++;
            }
            count = 0;
        }
    }

   public:
    std::vector<int> results;

    ///  @brief Emitter's constructor
    ///  @param *ff_loadbalancer The load balancer of the emitter
    ///  @param int              The initial (and maximum) number of Worker
    ///  @param int              The number of tasks to be computed
    EmitterFF(ff_loadbalancer *const lb, int nWorkers, int nTask, int time) : lb(lb) {
        this->nWorkers = nWorkers;
        this->nTask = nTask;
        this->maxWorkers = nWorkers;
        this->sleeping = 0;
        this->x = nWorkers;
        this->stopTime = time;
        std::cout << stopTime << std::endl;
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
        this->lastWorker = this->get_num_outchannels();
        lastUpdate = std::chrono::high_resolution_clock::now();

        return 0;
    }
    bool finish = false;

    Task<T, U> *svc(Task<T, U> *in) {
        int wid = lb->get_channel_id();
        //std::cout << "------------------ " << wid << " ------------------ " << std::endl;
        Task<T, U> *task = reinterpret_cast<Task<T, U> *>(in);

        /*  
            In this case we receive the task from the ExternalEmitter
            We must send the task to one of the available Workers.
         */
        if ((int)wid == -1) {
            int worker = getFirstActive();
            if (worker == -1) {
                inputTasks.push_back(task);
                return this->GO_ON;
            } else {
                sendTask(task, worker);
            }
        }

        // In this case we received the feedback from one of the workers
        if ((size_t)wid < this->get_num_outchannels()) {
            //std::cout << "ricevuto feedback da " << wid << std::endl;
            setFree(wid);
            received++;

            std::chrono::duration<double> elapsed = std::chrono::high_resolution_clock::now() - lastUpdate;
            int elapsedINT = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();

            if (elapsedINT > stopTime || first) {
                // wake up one or more worker based on the information of the feedback
                checkWakeUp(task);

                // send a sleep to one or more worker based on the information of the feedback
                checkSleep(task);
            }
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

                while (this->working < this->nWorkers && inputTasks.size() > 0) {
                    int index = getFirstActive();

                    if (index >= 0) {
                        Task<T, U> *inTask = reinterpret_cast<Task<T, U> *>(inputTasks.front());

                        inTask->workingThreads = this->nWorkers;

                        sendTask(inTask, index);

                        inputTasks.erase(inputTasks.begin());  /// <---------
                    }
                }
            }
        }

        // I have to check if i send all the tasks to the workers.
        if (eosReceived && inputTasks.size() == 0 && working == 0) {
            // std::cout << "STOP" << std::endl;
            /*
                In this case i send a wake up signal to all the workers
                and then i send an EOS to all the workers.
            */
            // unsigned int index = 0;
            // while (sleeping > 0 && index < sleepingWorkers.size()) {
            //     if (sleepingWorkers[index] == 1) {
            //         wakeUpWorker(index);
            //     }
            //     index++;
            // }
            for (int i = 0; i < this->maxWorkers; i++) {
                wakeUpWorker(i);
                //std::cout << "wake up " << i << std::endl;
                //this->ff_send_out_to(this->EOS, i);
            }
            this->broadcast_task(this->EOS);
            return this->EOS;
        } else {
            //std::cout << received << " " << sleeping + this->nWorkers << " " << eosReceived << " " << inputTasks.size() << " " << working << std::endl;
            return this->GO_ON;
        }
    }

    void svc_end() {
        // std::cout << "-----------Terminazione emitter-----------" << std::endl;
    }

    void eosnotify(ssize_t id) {
        if (id == -1) {
            eosReceived = true;
            //  std::cout << "EOS RECEIVED EMITTER" << std::endl;
        }
    }
};