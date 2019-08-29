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

   public:
    // In this vector I store the results
    std::vector<T> results;

    ///  @brief Collector's constructor
    CollectorFF(int tsGoal) {
        this->tsGoal = tsGoal;
    }

    void *svc(Task<T, U> *t) {
        /*
            The collector receives the task and put the result of the computation
            in the results vector.
        */
        results.push_back(t->result);
        delete (t);
        return this->GO_ON;
    }

    void svc_end() {
        //printf("----------------------Collector ending------------------------\n");
    }

    void eosnotify(ssize_t id) {
        ///std::cout << "EOS " << std::endl;
    }
};