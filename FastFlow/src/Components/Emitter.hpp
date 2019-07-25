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
    std::vector<Task *> inputTasks;
    int lastWorker = 0;
    int nTask;

    ///  @brief This function return the index of first active worker
    ///  @return The index of the first active worker
    int getFirstActive() {
        for (int i = this->lastWorker; i < activeWorkers.size(); i++) {
            if (activeWorkers[i] == 1) {
                this->lastWorker = i;
                return i;
            }
        }

        for (int i = 0; i < this->lastWorker; i++) {
            if (activeWorkers[i] == 1) {
                this->lastWorker = i;
                return i;
            }
        }
        return -1;
    }

    void setWorking(int index) {
        activeWorkers[index] = 0;
    }

    void setFree(int index) {
        activeWorkers[index] = 1;
    }

    void sendTask(Task *task, int worker) {
        lb->ff_send_out_to(task, worker);
        sent++;
    }

   public:
    std::vector<int> results;

    Emitter(ff_loadbalancer *const lb, int nWorkers, int nTask) : lb(lb) {
        this->nWorkers = nWorkers;
        this->nTask = nTask;
    }

    int svc_init() {
        activeWorkers.reserve(nWorkers);
        for (int i = 0; i < nWorkers; i++) {
            activeWorkers.push_back(1);
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
                setWorking(worker);
                sendTask(task, worker);
            }
        }
        // In this case we received the feedback from one of the workers
        else if ((size_t)wid < get_num_outchannels()) {
            if (inputTasks.size() > 0) {
                // inTask is the task that we estract from the temporary
                // vector where we store the tasks that we receive from
                // the external emitter that we can't send immediately to one worker.
                Task *inTask = reinterpret_cast<Task *>(inputTasks.front());
                sendTask(task, wid);
                inputTasks.erase(inputTasks.begin());
            } else {
                setFree(wid);
            }
        }

        if (sent == nTask)
            return EOS;
        else {
            return GO_ON;
        }
    };
};