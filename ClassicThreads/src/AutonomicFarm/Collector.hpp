// clang-format off
#include <unistd.h>
#include <iostream>
#include <utility>
#include <vector>
#include <boost/lockfree/queue.hpp>
// clang-format on

///  @brief Implementation of the collector of the autonomic farm
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

    ///  @brief Create a feedback with the number of workers that we want to use
    ///  @return The feedback to be sent
    Feedback createFeedback(int newNWorker) {
        Feedback f;
        if (newNWorker == 0) {
            f.newNumberOfWorkers = 1;
        } else if (newNWorker > this->maxWorkers) {
            f.newNumberOfWorkers = this->maxWorkers;
        } else {
            f.newNumberOfWorkers = newNWorker;
        }
        return f;
    }

    ///  @brief Function used to print some useful information
    ///  @param The task popped from the queue
    void debug(Task t) {
        std::chrono::duration<double> elapsed = t.endingTime - t.startingTime;
        int elapsedINT = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
        int TS = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count() / t.workingThreads;
        int newNWorker = round(float(elapsedINT) / this->tsGoal);
        std::cout << "elapsed " << elapsedINT << " tsGoal " << this->tsGoal << " actual TS " << TS << " current number of workers " << this->activeWorkers << " new number of workers: " << newNWorker << std::endl;
    }

    ///  @brief Constructor method of the Collector component
    ///  @param *InputQueue   The queue from which we extract the computed tasks
    ///  @param activeWorkers The initial number of active workers
    ///  @param TsGoal        The expected service time
    ///  @param FeedbackQueue The feedback queue used to send back a feedback to the emitter
   public:
    Collector(boost::lockfree::queue<Task> *inputQueue, int activeWorkers, int tsGoal, boost::lockfree::spsc_queue<Feedback> *feedbackQueue) {
        this->inputQueue = inputQueue;
        this->activeWorkers = activeWorkers;
        this->maxWorkers = activeWorkers;
        this->tsGoal = tsGoal;
        this->feedbackQueue = feedbackQueue;
    }

    ///  @brief Start the collector componentend.
    ///  @details
    ///  This function extracts a computed task from the input queue and computes
    ///  the best number of workers that will be used to compute the next tasks
    ///  based on the elapsed time of the popped task.
    ///  This function uses a spsc queue to communicate with the emitter to change
    ///  the number of workers
    ///  @return Void
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
                        int newNWorker = round(float(std::chrono::duration_cast<std::chrono::milliseconds>(t.endingTime - t.startingTime).count()) / this->tsGoal);
                        debug(t);

                        if (newNWorker != currentWorkers) {
                            currentWorkers = newNWorker;
                            Feedback f = createFeedback(newNWorker);
                            this->feedbackQueue->push(f);
                        }
                    }
                    accumulator.push_back(t.value);
                }
            }
        });
    }

    ///  @brief This function is used to join the thread
    ///  @return Void
    void join() {
        this->collectorThread.join();
    }
};