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

struct Worker : ff_monode_t<Task> {
   private:
    std::function<int(int x)> fun;
    int ID;
    int tsGoal;

    void setNewNWorker(Task *t) {
        std::chrono::duration<double> elapsed = t->endingTime - t->startingTime;
        int elapsedINT = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
        int TS = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count() / t->workingThreads;
        int newNWorker = round(float(elapsedINT) / this->tsGoal);
        std::cout << "Calcolato " << t->value << " Con " << t->workingThreads << " in " << elapsedINT << " myTS: " << TS << " Ideal TS " << this->tsGoal << " New NWorkers " << newNWorker << std::endl;

        t->newWorkingThreads = newNWorker;
    }

   public:
    Worker(std::function<int(int x)> fun, int ID, int tsGoal) {
        this->fun = fun;
        this->ID = ID;
        this->tsGoal = tsGoal;
    }

    Task *svc(Task *t) {
        t->startingTime = std::chrono::high_resolution_clock::now();
        t->result = fun(t->value);
        t->endingTime = std::chrono::high_resolution_clock::now();

        setNewNWorker(t);
        ff_send_out_to(t, 1);

        return t;
    }

    void svc_end() {
        printf("Going to sleep\n");
    }
};