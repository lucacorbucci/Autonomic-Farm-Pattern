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
class AutonomicFarm {
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
    ///  @param int Initial number of workers
    ///  @param int Expected service time
    ///  @param int Size of the input Vector
    ///  @param int Integer contained in the first part of the input vector
    ///  @param int Integer contained in the second part of the input vector
    ///  @param int Integer contained in the third part of the input vector
    ///  @param fun Function to be computed
    ///  @return Void
    AutonomicFarm(int nWorker, int tsGoal, int inputSize, U input1, U input2, U input3, std::function<T(U x)> fun) {
        this->nWorker = nWorker;
        this->tsGoal = tsGoal;
        this->inputSize = inputSize;
        this->n1 = input1;
        this->n2 = input2;
        this->n3 = input3;
        this->function = fun;
    }

    ///  @brief Start the execution of the farm
    int start() {
        // I create the vector with all the workers
        std::vector<ff_node *> w;
        for (int i = 0; i < nWorker; i++)
            w.push_back(new Worker<T, U>(this->function, i, tsGoal));

        // Fill the vector with input task
        std::vector<Task<T, U> *> inputVector = fillVector(inputSize, n1, n2, n3);
        ExternalEmitter<T, U> extEm(inputVector);
        ff_farm farm;
        Collector<T, U> *c = new Collector<T, U>(tsGoal);
        Emitter<T, U> *e = new Emitter<T, U>(farm.getlb(), nWorker, inputSize);

        farm.add_workers(w);
        farm.add_emitter(e);
        farm.wrap_around();

        farm.add_collector(c);

        //farm.wrap_around();
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
            //std::cout << item->result << std::endl;
            delete (item);
        }

        delete (c);
        delete (e);
        for (int i = 0; i < nWorker; i++)
            delete (w[i]);

        return 0;
    }
};