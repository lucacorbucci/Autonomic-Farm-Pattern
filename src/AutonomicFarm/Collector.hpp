// clang-format off
#include <unistd.h>
#include <iostream>
#include <utility>
#include <vector>
#include <boost/lockfree/queue.hpp>
// clang-format on

template <class T>
class Collector {
   private:
    boost::lockfree::queue<Task> *inputQueue;
    std::thread collectorThread;
    int activeWorkers;
    int tsGoal;
    std::vector<int> accumulator;

   public:
    /*
    */
    Collector(boost::lockfree::queue<Task> *inputQueue, int activeWorkers, int tsGoal) {
        this->inputQueue = inputQueue;
        this->activeWorkers = activeWorkers;
        this->tsGoal = tsGoal;
    }

    /*
        
    */
    void start() {
        std::cout << "Collector avviato" << std::endl;
        this->collectorThread = std::thread([=] {
            int counter = 0;
            while (true) {
                Task t;
                //I have to check if the queue is empty or not.
                if (inputQueue->pop(t)) {
                    if (t.value == -1) {
                        counter++;
                        if (counter == this->activeWorkers) break;
                    } else {
                        this->activeWorkers = t.workingThreads;
                        std::chrono::duration<double> elapsed = t.endingTime - t.startingTime;
                        //std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count() << std::endl;

                        std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count() << " " << this->tsGoal << " " << std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count() / this->tsGoal << " current " << this->activeWorkers << std::endl;
                    }
                    accumulator.push_back(t.value);
                }
            }
        });
    }

    void
    join() {
        this->collectorThread.join();
    }
};