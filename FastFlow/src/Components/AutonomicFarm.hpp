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

using namespace ff;

class AutonomicFarm {
   private:
    int nWorker;
    int tsGoal;
    int inputSize;
    int n1;
    int n2;
    int n3;
    std::function<int(int x)> function;

    std::vector<Task *>
    fillVector(int inputSize, int n1, int n2, int n3) {
        std::vector<Task *> inputVector;
        inputVector.reserve(inputSize);
        for (int i = 0; i < inputSize; i++) {
            Task *task = new Task();
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
    AutonomicFarm(int nWorker, int tsGoal, int inputSize, int n1, int n2, int n3, std::function<int(int x)> fun) {
        this->nWorker = nWorker;
        this->tsGoal = tsGoal;
        this->inputSize = inputSize;
        this->n1 = n1;
        this->n2 = n2;
        this->n3 = n3;
        this->function = fun;
    }

    int start() {
        std::vector<ff_node *> w;
        for (int i = 0; i < nWorker; i++)
            w.push_back(new Worker(this->function, i, tsGoal));

        // Fill the vector with input task
        std::vector<Task *> inputVector = fillVector(inputSize, n1, n2, n3);
        ExternalEmitter extEm(inputVector);
        std::cout << "Vector size " << inputVector.size() << std::endl;
        ff_farm farm;
        Collector *c = new Collector(tsGoal);
        Emitter *e = new Emitter(farm.getlb(), nWorker, inputSize);

        farm.add_workers(w);
        farm.add_emitter(e);
        farm.wrap_around();

        farm.add_collector(c);

        //farm.wrap_around();
        //farm.run_and_wait_end();
        farm.getlb()->waitlb();
        ff_Pipe<> pipe(extEm, farm);

        if (pipe.run_then_freeze() < 0) {
            error("running pipe\n");

            return -1;
        }

        pipe.wait_freezing();

        pipe.wait();

        std::vector<int> results = c->results;

        for (auto item : results) {
            std::cout << item << std::endl;
        }
        return 0;
    }
};
