// clang-format off
#include <unistd.h>
#include <iostream>
#include <utility>
#include <vector>

// clang-format on

///  @brief Implementation of the collector of the autonomic farm
///  @detail Typename T is used for as output type of the function that
///  the worker will compute. Typename U is input as output type of the function
///  that the worker will compute.
template <class T, class U>
class CollectorSQ {
   private:
    SafeQueue<Task<T, U> *> *inputQueue;
    std::thread collectorThread;
    std::vector<T> accumulator;
    SafeQueue<Feedback *> *feedbackQueue;
    int activeWorkers;
    int tsGoal;
    int maxWorkers;
    int count = 0;
    int x;
    int currentWorkers;

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
    void debug(Task<T, U> *t) {
        std::chrono::duration<double> elapsed = t->endingTime - t->startingTime;
        int elapsedINT = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
        int TS = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count() / t->workingThreads;
        int newNWorker = round(float(elapsedINT) / this->tsGoal);
        //std::cout << TS << std::endl;
        //std::cout << "Value " << t->value << " elapsed " << elapsedINT << " tsGoal " << this->tsGoal << " actual TS " << TS << " current number of workers " << t->workingThreads << " new number of workers: " << newNWorker << std::endl;
    }

    ///  @brief Constructor method of the Collector component
    ///  @param *InputQueue   The queue from which we extract the computed tasks
    ///  @param activeWorkers The initial number of active workers
    ///  @param TsGoal        The expected service time
    ///  @param FeedbackQueue The feedback queue used to send back a feedback to the emitter
   public:
    CollectorSQ(SafeQueue<Task<T, U> *> *inputQueue, int activeWorkers, int tsGoal, SafeQueue<Feedback *> *feedbackQueue) {
        this->inputQueue = inputQueue;
        this->activeWorkers = activeWorkers;
        this->maxWorkers = activeWorkers;
        this->tsGoal = tsGoal;
        this->feedbackQueue = feedbackQueue;
        x = activeWorkers;
        this->currentWorkers = activeWorkers;
    }

    void sendFeedback(int newNWorker) {
        if (newNWorker != currentWorkers) {
            if (count == 0) {
                x = newNWorker;
                count = 1;
            } else {
                newNWorker == x ? count++ : count = 0;
            }

            if (count == 2) {
                currentWorkers = newNWorker;
                Feedback f = createFeedback(newNWorker);
                this->feedbackQueue->safe_push(&f);
                count = 0;
            }
        }
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
        this->collectorThread = std::thread([=] {
            int counter = 0;
            int c = 0;
            while (true) {
                void *tmpTask;
                //I have to check if the queue is empty or not.
                if (!inputQueue->isEmpty()) {
                    if (tmpTask = inputQueue->safe_pop()) {
                        Task<T, U> *t = reinterpret_cast<Task<T, U> *>(tmpTask);

                        if (t->end == -1) {
                            counter++;
                            delete (t);
                            if (counter == this->activeWorkers) break;
                        } else {
                            int newNWorker = round(float(std::chrono::duration_cast<std::chrono::milliseconds>(t->endingTime - t->startingTime).count()) / this->tsGoal);
                            debug(t);
                            sendFeedback(newNWorker);

                            accumulator.push_back(t->result);
                            delete (t);
                        }
                    }
                }
            }
            for (T x : accumulator) {
                std::cout << x << std::endl;
            }
        });
    }

    ///  @brief This function is used to join the thread
    ///  @return Void
    void join() {
        this->collectorThread.join();
    }
};