/*
    author: Luca Corbucci
    student number: 516450
*/

// clang-format off
#include <unistd.h>
#include <iostream>
#include <utility>
#include <vector>
#include "../Utils/Task.hpp"
#include "../Utils/Feedback.hpp"
#include <ff/ff.hpp>
#include <ff/farm.hpp>
#include "./Worker.hpp"
#include "./Collector.hpp"
#include "./Emitter.hpp"
#include "./ExternalEmitter.hpp"
#include <cstdio>
#include <math.h>
// clang-format on

///  @brief Implementation of the Autonomic Farm Pattern
template <typename T, typename U>
class AutonomicFarmFF {
   private:
    // Initial (and maximum) number of worker
    int nWorker;
    // Expected service time
    int tsGoal;
    // Size of the vector with imput tasks
    int inputSize;
    // Input of the vector
    U n1;
    U n2;
    U n3;
    // Function to be computed
    std::function<T(U x)> function;
    int time;
    std::string debug;
    ///  @brief Send a sleep signal to a thread
    ///  @param int Size of the input Vector
    ///  @param int Integer contained in the first part of the input vector
    ///  @param int Integer contained in the second part of the input vector
    ///  @param int Integer contained in the third part of the input vector
    ///  @return std::vector<Task *> Filled vector
    std::vector<Task<T, U> *> fillVector(int inputSize, U n1, U n2, U n3) {
        std::vector<Task<T, U> *> inputVector;
        inputVector.reserve(inputSize);
        for (int i = 0; i < inputSize; i++) {
            Task<T, U> *task = new Task<T, U>();
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

   public:
    ///  @brief AutonomicFarm's constructor
    ///  @param int     Initial number of workers
    ///  @param int     Expected service time
    ///  @param int     Size of the input Vector
    ///  @param int     Integer contained in the first part of the input vector
    ///  @param int     Integer contained in the second part of the input vector
    ///  @param int     Integer contained in the third part of the input vector
    ///  @param fun     Function to be computed
    ///  @param int     Time: This is time that we have to wait to change the number of workers of the farm
    ///  @param string  This is used to print some informations during the execution of the farm
    ///  @return Void
    AutonomicFarmFF(int nWorker, int tsGoal, int inputSize, U input1, U input2, U input3, std::function<T(U x)> fun, int time, std::string debug) {
        this->nWorker = nWorker;
        this->tsGoal = tsGoal;
        this->inputSize = inputSize;
        this->n1 = input1;
        this->n2 = input2;
        this->n3 = input3;
        this->function = fun;
        this->time = time;
        this->debug = debug;
    }

    ///  @brief Start the execution of the farm
    int start() {
        // I create the vector with all the workers
        std::vector<ff_node *> w;
        for (int i = 0; i < nWorker; i++)
            w.push_back(new WorkerFF<T, U>(this->function, tsGoal));

        // Fill the vector with input task
        std::vector<Task<T, U> *> inputVector = fillVector(inputSize, n1, n2, n3);
        ExternalEmitterFF<T, U> extEm(inputVector);
        ff_farm farm;
        CollectorFF<T, U> *c = new CollectorFF<T, U>(tsGoal, debug);
        EmitterFF<T, U> *e = new EmitterFF<T, U>(farm.getlb(), nWorker, inputSize, time);

        farm.add_workers(w);
        farm.add_emitter(e);
        farm.wrap_around();

        farm.add_collector(c);

        farm.wrap_around();
        //farm.run_and_wait_end();
        farm.getlb()->waitlb();
        ff_Pipe<> pipe(extEm, farm);
        ffTime(START_TIME);
        if (pipe.run_then_freeze() < 0) {
            error("running pipe\n");

            return -1;
        }

        pipe.wait_freezing();

        pipe.wait();
        ffTime(STOP_TIME);
        std::cout << "Time: " << ffTime(GET_TIME) << " (ms)\n";

        /*
            Get the output from the collector and print it
        */
        std::vector<Task<T, U> *> results = c->results;

        for (auto item : results) {
            if (debug == "results") {
                std::cout << item->result << std::endl;
            }
            delete (item);
        }

        delete (c);
        delete (e);
        for (int i = 0; i < nWorker; i++)
            delete (w[i]);

        return 0;
    }
};