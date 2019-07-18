// clang-format off
#include <unistd.h>
#include <iostream>
#include <utility>
#include <vector>
#include "Worker.cpp"
#include "Emitter.cpp"
#include "Collector.cpp"
#include <mutex>
#include "utimer.hpp"
#include <boost/lockfree/spsc_queue.hpp>
// clang-format on

int fib(int x) {
    if ((x == 1) || (x == 0)) {
        return (x);
    } else {
        return (fib(x - 1) + fib(x - 2));
    }
}


int main(){
	std::cout  << fib(45)  << std::endl;
	return 0;
}
