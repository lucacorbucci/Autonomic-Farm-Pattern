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

class Emitter : public ff_node {
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

    void *svc(void *in) {
        int wid = lb->get_channel_id();
        if (wid == -2) {
            //std::cout << "invio" << std::endl;
            Task *task = this->tasks.at(index);
            index++;
            int worker = getFirstActive();
            //std::cout << index << std::endl;
            //std::cout << worker << std::endl;
            lb->ff_send_out_to(task, worker);
            sent++;
            // std::cout << sent << std::endl;
            if (sent == tasks.size())
                return EOS;
            else
                return GO_ON;
        } else {
            int x = *(int *)in;
            count++;
            return GO_ON;
        }

        // Per addormentare un thread
        // lb->freeze(0);
        // lb->ff_send_out_to(GO_OUT, 0);
        // lb->wait_freezing(0);
        // lb->freeze(1);
        // lb->ff_send_out_to(GO_OUT, 1);
        // lb->wait_freezing(1);
    };

    // void eosnotify(ssize_t id) {
    //     if (count == 4 && sent == 4) {
    //         std::cout << "ciao" << std::endl;
    //         eos_received = true;

    //         //lb->broadcast_task(EOS);
    //     }
    // }
};

class Worker : public ff_node {
   private:
    std::function<int(int x)> fun;

   public:
    Worker(std::function<int(int x)> fun) {
        this->fun = fun;
    }

    void *svc(void *t) {
        //std::cout << "Worker" << std::endl;
        Task *task = (Task *)t;
        std::cout << task->value << std::endl;
        std::cout << "worker" << std::endl;

        // std::cout << "task.value" << task.value << std::endl;
        // std::cout << "task value " << task->value << std::endl;
        // task.startingTime = std::chrono::high_resolution_clock::now();
        // std::cout << this->fun(10) << std::endl;
        // task.endingTime = std::chrono::high_resolution_clock::now();

        return new int(5);
    }

    void svc_end() {
        printf("Worker ending\n");
    }
};

class Collector : public ff_node {
   public:
    std::vector<int> results;

    void *svc(void *t) {
        std::cout << "Collector" << std::endl;
        //Task *task = (Task *)t;
        int v = *(int *)t;
        std::cout << v << std::endl;

        //std::cout << task->value << std::endl;

        //std::cout << "collector " << task->value << std::endl;
        results.push_back(v);
        return GO_ON;
        // return new int(100);
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
    Collector *c = new Collector();
    Emitter *e = new Emitter(farm.getlb(), inputVector);

    farm.add_workers(w);
    farm.add_emitter(e);
    farm.add_collector(c);

    //farm.wrap_around();
    farm.run_and_wait_end();
    farm.getlb()->waitlb();

    std::vector<int> results = c->results;

    for (auto item : results) {
        std::cout << item << std::endl;
    }

    return 0;
}