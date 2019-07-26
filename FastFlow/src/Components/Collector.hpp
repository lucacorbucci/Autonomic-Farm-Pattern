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
struct Collector : ff_minode_t<Task, void> {
   private:
    // Expected service time
    int tsGoal;

   public:
    // In this vector I store the results of the service time
    std::vector<int> results;

    ///  @brief Collector's constructor
    Collector(int tsGoal) {
        this->tsGoal = tsGoal;
    }

    void *svc(Task *t) {
        /*
            The collector receives the task and put the result of the computation
            in the results vector.
        */
        results.push_back(t->result);
        return GO_ON;
    }

    void svc_end() {
        //printf("Collector ending\n");
    }
};