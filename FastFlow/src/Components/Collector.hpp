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

struct Collector : ff_minode_t<Task, void> {
   private:
    int tsGoal;

   public:
    std::vector<int> results;

    Collector(int tsGoal) {
        this->tsGoal = tsGoal;
    }

    void *svc(Task *t) {
                results.push_back(t->result);
        return GO_ON;
    }

    void svc_end() {
        printf("Collector ending\n");
    }
};