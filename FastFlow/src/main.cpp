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
// clang-format on

using namespace ff;

struct Emitter : ff_node_t<Feedback, Task> {
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

    ///  @brief This function return the index of first active worker
    ///  @return The index of the first active worker
    int getFirstActive() {
        int index = 0;
        for (int x : activeWorkers) {
            if (x == 1) {
                return index;
            } else
                index++;
        }
        return -1;
    }

    int getFirstSleeping() {
        int index = 0;
        int f = 0;
        for (int x : sleepingWorkers) {
            if (x == 1) {
                return index;
            } else
                index++;
        }
        return -1;
    }

   public:
    Emitter(ff_loadbalancer *const lb, std::vector<Task *> tasks) : lb(lb) {
        this->tasks = tasks;
    }

    int svc_init() {
        nWorkers = lb->getnworkers();
        sleepingWorkers.reserve(nWorkers);
        activeWorkers.reserve(nWorkers);
        for (int i = 0; i < nWorkers; i++) {
            sleepingWorkers.push_back(0);
            activeWorkers.push_back(1);
        }

        return 0;
    }

    Task *svc(Feedback *in) {
        int wid = lb->get_channel_id();

        // In this case we received the feedback from the collector
        // and we must change the parallelism degree.
        if (in != nullptr) {
            //Feedback *x = (Feedback *)in;
            std::cout << in->newNumberOfWorkers << std::endl;
            count++;
        }

        // We send the task to one of the available workers
        std::cout << "invio" << std::endl;
        Task *task = this->tasks.at(index);
        index++;
        int worker = getFirstActive();
        lb->ff_send_out_to(task, worker);
        sent++;
        if (sent == tasks.size())
            return EOS;
        else
            return GO_ON;
    };
};

struct Worker : ff_node_t<Task> {
   private:
    std::function<int(int x)> fun;

   public:
    Worker(std::function<int(int x)> fun) {
        this->fun = fun;
    }

    Task *svc(Task *t) {
        return t;
    }

    void svc_end() {
        printf("Worker ending\n");
    }
};

struct Collector : ff_minode_t<Task, Feedback> {
   public:
    std::vector<int> results;

    Feedback *svc(Task *t) {
        results.push_back(t->value);
        //return GO_ON;
        Feedback *f = new Feedback();
        f->newNumberOfWorkers = 100;
        return f;
    }

    void svc_end() {
        printf("Collector ending\n");
    }
};

int fib(int x) {
    if ((x == 1) || (x == 0)) {
        return (x);
    } else {
        return (fib(x - 1) + fib(x - 2));
    }
}

int main(int argc, char *argv[]) {
    int nworkers = 1;
    std::vector<ff_node *> w;
    for (int i = 0; i < nworkers; ++i)
        w.push_back(new Worker(fib));

    std::vector<Task *> inputVector;
    for (int i = 0; i < 10; i++) {
        Task *task = new Task();
        task->value = 10;
        inputVector.push_back(task);
    }

    ff_farm farm;
    Collector c;
    Emitter *e = new Emitter(farm.getlb(), inputVector);

    farm.add_workers(w);
    farm.add_emitter(e);
    farm.add_collector(&c);

    farm.wrap_around();
    farm.run_and_wait_end();
    farm.getlb()->waitlb();

    std::vector<int> results = c.results;

    for (auto item : results) {
        std::cout << item << std::endl;
    }

    return 0;
}