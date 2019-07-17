// clang-format off
#include <unistd.h>
#include <iostream>
#include <utility>
#include <vector>
#include "Worker.cpp"
#include "Emitter.cpp"
#include <mutex>
// clang-format on

void f() {
    std::cout << "Hello World!" << std::endl;
}

int main() {
    SafeQueue<int> inputQueue;

    Emitter<int> e{&inputQueue};
    Worker<int, int>
        w{
            f, &inputQueue};

    e.start();
    w.start();

    return 0;
}