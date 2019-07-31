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

///  @brief Implementation of the "external emitter"
template <typename T, typename U>
struct ExternalEmitter : ff_node_t<Task<T, U>, void> {
    std::vector<Task<T, U> *> inputTasks;

    ExternalEmitter(std::vector<Task<T, U> *> inputTasks) : inputTasks(inputTasks) {}

    void *svc(Task<T, U> *) {
        for (unsigned int i = 0; i < inputTasks.size(); i++) {
            this->ff_send_out(inputTasks[i]);
        }

        return this->EOS;
    }
};