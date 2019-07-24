// clang-format off
#include <unistd.h>
#include <iostream>
#include <utility>
#include <vector>
#include <ff/ff.hpp>
#include <ff/farm.hpp>
#include "./Utils/Task.hpp"
#include "./Utils/Feedback.hpp"
#include <cstdio>
#include <math.h>
// clang-format on

using namespace ff;

// first stage

struct Seq : ff_node_t<Task, void> {
    std::vector<Task *> tasks;

    Seq(std::vector<Task *> tasks) : tasks(tasks) {}

    void *svc(Task *) {
        for (int i = 1; i <= tasks.size(); i++) {
            ff_send_out(tasks[i]);
        }

        return EOS;
    }
};

struct Emitter : ff_monode_t<Task, Task> {
   private:
    ff_loadbalancer *const lb;
    bool eos_received;
    int onthefly = 0;
    int count = 0;
    int sent = 0;
    int nWorkers;
    int index = 0;
    std::vector<int> sleepingWorkers;
    std::vector<int> activeWorkers;
    std::vector<Task *> tasks;
    int lastWorker = 0;

    ///  @brief This function return the index of first active worker
    ///  @return The index of the first active worker
    int getFirstActive() {
        int index = 0;
        for (int i = lastWorker + 1; i < activeWorkers.size(); i++) {
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

    int getFirstSleeping() {
        int f = 0;
        for (int i = 0; i < activeWorkers.size(); i++) {
            if (activeWorkers[i] == 1) {
                return i;
            }
        }
        return -1;
    }

    void printVec() {
        for (int x : activeWorkers) {
            std::cout << x << std::endl;
        }
    }

    void setWorking(int index) {
        activeWorkers[index] = 0;
    }

    void setFree(int index) {
        //std::cout << "risvegliato " << index << std::endl;
        activeWorkers[index] = 1;
    }

   public:
    Emitter(ff_loadbalancer *const lb, std::vector<Task *> tasks, int nWorkers) : lb(lb) {
        this->tasks = tasks;
        this->nWorkers = nWorkers;
        this->lastWorker = get_num_outchannels();
    }

    int svc_init() {
        sleepingWorkers.reserve(nWorkers);
        activeWorkers.reserve(nWorkers);
        for (int i = 0; i < nWorkers; i++) {
            sleepingWorkers.push_back(0);
            activeWorkers.push_back(1);
        }
        return 0;
    }

    Task *svc(Task *in) {
        int wid = lb->get_channel_id();
        std::cout << wid << std::endl;
        // In this case we received the feedback from the collector
        // and we must change the parallelism degree.
        if ((size_t)wid < get_num_outchannels()) {
            Feedback *feedback = reinterpret_cast<Feedback *>(in);

            if (feedback->workerID > 0) {
                setFree(wid);
                return GO_ON;
            } else {
                count++;
            }
        }

        // We send the task to one of the available workers
        int worker = getFirstActive();

        if (worker == -1) {
            return GO_ON;
        } else {
            // if ((size_t)wid == -1) {
            //     Task *task = reinterpret_cast<Task *>(in);
            //     task->workingThreads = nWorkers;
            //     index++;
            //     setWorking(worker);

            //     lb->ff_send_out_to(task, worker);
            //     sent++;
            //     if (sent == tasks.size())
            //         return EOS;
            //     else {
            //         return GO_ON;
            //     }
            // }
        }
    };
};

struct Worker : ff_monode_t<Task> {
   private:
    std::function<int(int x)> fun;
    int ID;

   public:
    Worker(std::function<int(int x)> fun, int ID) {
        this->fun = fun;
        this->ID = ID;
    }

    Task *svc(Task *t) {
        std::cout << "Worker " << ID << " Is Working" << std::endl;
        t->startingTime = std::chrono::high_resolution_clock::now();
        fun(t->value);

        t->endingTime = std::chrono::high_resolution_clock::now();
        Feedback *f = new Feedback();

        ff_send_out_to(f, 0);
        //ff_send_out_to(t, 1);
        return t;
    }

    void svc_end() {
        // printf("Worker ending\n");
    }
};

struct Collector : ff_minode_t<Task, Feedback> {
   private:
    int tsGoal;

   public:
    std::vector<int> results;

    Collector(int tsGoal) {
        this->tsGoal = tsGoal;
    }

    Feedback *svc(Task *t) {
        std::chrono::duration<double> elapsed = t->endingTime - t->startingTime;
        int elapsedINT = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
        int TS = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count() / t->workingThreads;
        int newNWorker = round(float(elapsedINT) / this->tsGoal);

        results.push_back(t->value);
        Feedback *f = new Feedback();
        f->newNumberOfWorkers = 100;
        return f;
    }

    void svc_end() {
        // printf("Collector ending\n");
    }
};

int fib(int x) {
    if ((x == 1) || (x == 0)) {
        return (x);
    } else {
        return (fib(x - 1) + fib(x - 2));
    }
}

/* 
------------------------------------------------------------------------
*/

std::vector<Task *> fillVector(int inputSize, int n1, int n2, int n3) {
    std::vector<Task *> inputVector;
    inputVector.reserve(inputSize);
    for (int i = 0; i < inputSize; i++) {
        Task *task = new Task();
        if (i > 2 * (inputSize / 3))
            task->value = n3;
        else if (i > (inputSize / 3)) {
            task->value = n2;
        } else {
            task->value = n1;
        }
        inputVector.push_back(task);
    }
    return inputVector;
}

int main(int argc, char *argv[]) {
    if (argc >= 6) {
        int nWorker = atoi(argv[1]);
        int tsGoal = atoi(argv[2]);
        int inputSize = atoi(argv[3]);
        int input1 = atoi(argv[4]);
        int input2 = atoi(argv[5]);
        int input3 = atoi(argv[6]);

        std::vector<ff_node *> w;
        for (int i = 0; i < nWorker; i++)
            w.push_back(new Worker(fib, i));

        // Fill the vector with input task
        std::vector<Task *> inputVector = fillVector(inputSize, input1, input2, input3);
        Seq seq(inputVector);

        ff_farm farm;
        Collector *c = new Collector(tsGoal);
        Emitter *e = new Emitter(farm.getlb(), inputVector, nWorker);

        farm.add_workers(w);
        farm.add_emitter(e);
        farm.wrap_around();

        farm.add_collector(c);

        //farm.wrap_around();
        //farm.run_and_wait_end();
        farm.getlb()->waitlb();
        ff_Pipe<> pipe(seq, farm);

        if (pipe.run_then_freeze() < 0) {
            error("running pipe\n");

            return -1;
        }

        pipe.wait_freezing();

        pipe.wait();

        std::vector<int> results = c->results;

        for (auto item : results) {
            std::cout << item << std::endl;
        }
    } else {
        std::cout << argv[0] << " Usage: nWorker, tsGoal" << std::endl;
    }
    return 0;
}