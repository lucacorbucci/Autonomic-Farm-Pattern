/*
    author: Luca Corbucci
    student number: 516450
*/

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

///  @brief Implementation of the collector of the Autonomic Farm
template <typename T, typename U>
struct CollectorFF : ff_minode_t<Task<T, U>, void> {
   private:
    // Expected service time
    int tsGoal;
    std::string debugStr;

    ///  @brief Change the number of workers that we want to have in the farm
    ///  based on the elapsed time and the TsGoal.
    ///  @param Task* Task that I use as a feedback where I store the new number of workers.
    void setNewNWorker(Task<T, U> *t) {
        std::chrono::duration<double> elapsed = t->endingTime - t->startingTime;
        int elapsedINT = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
        int TS = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count() / t->workingThreads;
        int newNWorker = round(float(elapsedINT) / this->tsGoal);
        if (debugStr == "true") {
            std ::cout << "Calcolato " << t->value << " Con " << t->workingThreads << " in " << elapsedINT << " myTS: " << TS << " Ideal TS " << this->tsGoal << " New NWorkers " << newNWorker << " da " << this->get_my_id() << std::endl;
        } else if (debugStr == "ts") {
            std::cout << TS << " " << t->workingThreads << std::endl;
        }
        t->newWorkingThreads = newNWorker;
    }

   public:
    // In this vector I store the results
    std::vector<Task<T, U> *> results;

    ///  @brief Collector's constructor
    CollectorFF(int tsGoal, std::string debugStr) {
        this->tsGoal = tsGoal;
        this->debugStr = debugStr;
    }

    void *svc(Task<T, U> *t) {
        /*
            The collector receives the task and put the result of the computation
            in the results vector.
        */
        setNewNWorker(t);
        this->ff_send_out(t);
        results.push_back(t);
        return this->GO_ON;
    }

    void svc_end() {
        //printf("----------------------Collector ending------------------------\n");
    }

    void eosnotify(ssize_t id) {
        ///std::cout << "EOS " << std::endl;
    }
};