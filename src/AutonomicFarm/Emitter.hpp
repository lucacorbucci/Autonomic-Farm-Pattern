// clang-format off
#include <unistd.h>
#include <iostream>
#include <utility>
#include <vector>
#include <boost/lockfree/spsc_queue.hpp>
// clang-format on

template <class T>
class Emitter {
   private:
    std::vector<boost::lockfree::spsc_queue<Task> *> outputQueue;
    std::thread emitterThread;

   public:
    Emitter(std::vector<boost::lockfree::spsc_queue<Task> *> outputQueue) {
        this->outputQueue = outputQueue;
    }

    long mod(long a, long b) { return (a % b + b) % b; }

    void start() {
        std::cout << "Emitter avviato" << std::endl;
        this->emitterThread = std::thread([=] {
            int i = 100;
            auto index = 0;
            while (i > 0) {
                Task task;

                index = mod(i, outputQueue.size());
                if (i > 70)
                    task.value = 40;
                else if (i > 30)
                    task.value = 43;
                else
                    task.value = 45;
                outputQueue[index]->push(task);

                i--;
            }
            for (int y = 0; y < outputQueue.size(); y++) {
                Task task;
                task.value = -1;
                outputQueue[y]->push(task);
            }
        });
    }

    void join() {
        this->emitterThread.join();
    }
};