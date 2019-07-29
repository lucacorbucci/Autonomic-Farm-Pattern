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

///  @brief Implementation of the Worker of the farm
struct Worker : ff_monode_t<Task> {
   private:
    // Function computed by the worker
    std::function<int(int x)> fun;
    // ID of the worker
    int ID;
    int tsGoal;

    ///  @brief Change the number of workers that we want to have in the farm
    ///  based on the elapsed time and the TsGoal.
    ///  @param Task* Task that I use as a feedback where I store the new number of workers.
    void setNewNWorker(Task *t) {
        std::chrono::duration<double> elapsed = t->endingTime - t->startingTime;
        int elapsedINT = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
        int TS = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count() / t->workingThreads;
        int newNWorker = round(float(elapsedINT) / this->tsGoal);
        //std ::cout << "Calcolato " << t->value << " Con " << t->workingThreads << " da " << get_my_id() << " in " << elapsedINT << " myTS: " << TS << " Ideal TS " << this->tsGoal << " New NWorkers " << newNWorker << std::endl;

        t->newWorkingThreads = newNWorker;
    }

   public:
    ///  @brief Worker's constructor
    ///  @param fun Function to be computed
    ///  @param int Worker's ID
    ///  @param int Expected service time

    Worker(std::function<int(int x)> fun, int ID, int tsGoal) {
        this->fun = fun;
        this->ID = ID;
        this->tsGoal = tsGoal;
    }

    Task *svc(Task *t) {
        /*
            I store in the task structure the starting time of the worker
            and the ending time. Then i send the task to the collector
            and also to the emitter as a feedback.
        */
        t->startingTime = std::chrono::high_resolution_clock::now();
        t->result = fun(t->value);
        t->endingTime = std::chrono::high_resolution_clock::now();

        setNewNWorker(t);
        ff_send_out_to(t, 1);
        ff_send_out_to(t, 0);

        return GO_ON;
    }

    void svc_end() {
        // std::cout << "Going to sleep " << get_my_id() << std::endl;
    }
};