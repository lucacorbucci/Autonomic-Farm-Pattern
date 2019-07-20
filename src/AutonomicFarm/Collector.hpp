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
    std::vector<int> accumulator;
    boost::lockfree::spsc_queue<Feedback> *feedbackQueue;
    int activeWorkers;
    int tsGoal;
    int maxWorkers;

   public:
    /*
    */
    Collector(boost::lockfree::queue<Task> *inputQueue, int activeWorkers, int tsGoal, boost::lockfree::spsc_queue<Feedback> *feedbackQueue) {
        this->inputQueue = inputQueue;
        this->activeWorkers = activeWorkers;
        this->maxWorkers = activeWorkers;
        this->tsGoal = tsGoal;
        this->feedbackQueue = feedbackQueue;
    }

    /*
        
    */
    void start() {
        std::cout << "Collector avviato" << std::endl;
        this->collectorThread = std::thread([=] {
            int counter = 0;
            int c = 0;
            int currentWorkers = this->activeWorkers;
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
                        int elapsedINT = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
                        //std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count() << std::endl;
                        int TS = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count() / this->activeWorkers;
                        int newNWorker = round(float(elapsedINT) / this->tsGoal);

                        //std::cout << "elapsed " << elapsedINT << " tsGoal " << this->tsGoal << " actual TS " << TS << " current number of workers " << this->activeWorkers << " new number of workers: " << newNWorker << std::endl;
                        Feedback f;
                        if (newNWorker != currentWorkers) {
                            currentWorkers = newNWorker;
                            // std::cout << "feedback emitted "
                            //           << " " << newNWorker << " " << currentWorkers << std::endl;
                            if (newNWorker == 0) {
                                f.newNumberOfWorkers = 1;
                            } else if (newNWorker > this->maxWorkers) {
                                f.newNumberOfWorkers = this->maxWorkers;
                            } else {
                                f.newNumberOfWorkers = newNWorker;
                            }
                            this->feedbackQueue->push(f);
                        }
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