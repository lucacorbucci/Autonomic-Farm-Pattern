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

///  @brief Implementation of the Worker of the farm
template <typename T, typename U>
struct WorkerFF : ff_monode_t<Task<T, U>> {
   private:
    // Function computed by the worker
    std::function<T(U x)> fun;
    int tsGoal;

   public:
    ///  @brief Worker's constructor
    ///  @param fun Function to be computed
    ///  @param int Worker's ID
    ///  @param int Expected service time
    ///  @param debugStr String used to print some informations during the execution of the farm
    WorkerFF(std::function<T(U x)> fun, int tsGoal) {
        this->fun = fun;
        this->tsGoal = tsGoal;
    }

    Task<T, U> *svc(Task<T, U> *t) {
        /*
            I store in the task structure the starting time of the worker
            and the ending time. Then i send the task to the collector
            and also to the emitter as a feedback.
        */
        t->startingTime = std::chrono::high_resolution_clock::now();
        t->result = fun(t->value);
        t->endingTime = std::chrono::high_resolution_clock::now();

        // Send task to collector
        this->ff_send_out_to(t, 1);
        // Send ack to emitter
        this->ff_send_out_to(t, 0);
        return this->GO_ON;
    }

    void svc_end() {
        //std::cout << "Going to sleep " << this->ID << std::endl;
    }

    void eosnotify(ssize_t id) {
        //std::cout << "EOS " << this->ID << std::endl;
    }
};