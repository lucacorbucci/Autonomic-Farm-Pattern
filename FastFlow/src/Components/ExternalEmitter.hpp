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

struct ExternalEmitter : ff_node_t<Task, void> {
    std::vector<Task *> inputTasks;

    ExternalEmitter(std::vector<Task *> inputTasks) : inputTasks(inputTasks) {}

    void *svc(Task *) {
        for (int i = 0; i < inputTasks.size(); i++) {
            ff_send_out(inputTasks[i]);
        }

        return EOS;
    }
};
