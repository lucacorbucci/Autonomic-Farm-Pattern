/*
    author: Luca Corbucci
    student number: 516450
*/

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
    std::atomic<int> *timeEmitter;
    std::atomic<int> *timeCollector;

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
        // std::cout << "Value " << t->value << " elapsed " << elapsedINT << " tsGoal " << this->tsGoal << " actual TS " << TS << " current number of workers " << t->workingThreads << " new number of workers: " << newNWorker << std::endl;
    }

    ///  @brief Constructor method of the Collector component
    ///  @param *InputQueue   The queue from which we extract the computed tasks
    ///  @param activeWorkers The initial number of active workers
    ///  @param TsGoal        The expected service time
    ///  @param FeedbackQueue The feedback queue used to send back a feedback to the emitter
    ///  @param timeEmitter   Emitter's service time
    ///  @param timeCollector Collector's service time
   public:
    CollectorSQ(SafeQueue<Task<T, U> *> *inputQueue, int activeWorkers, int tsGoal, SafeQueue<Feedback *> *feedbackQueue, std::atomic<int> *timeEmitter, std::atomic<int> *timeCollector) {
        this->inputQueue = inputQueue;
        this->activeWorkers = activeWorkers;
        this->maxWorkers = activeWorkers;
        this->tsGoal = tsGoal;
        this->feedbackQueue = feedbackQueue;
        x = activeWorkers;
        this->currentWorkers = activeWorkers;
        this->timeEmitter = timeEmitter;
        this->timeCollector = timeCollector;
    }

    ///  @brief This method is used to create and send the feedback from the collector to the emitter
    ///  @param newNWorker  The new number of workers
    void sendFeedback(int newNWorker) {
        Feedback f = createFeedback(newNWorker);
        this->feedbackQueue->safe_push(&f);
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
        // std::cout << "collector avviato" << std::endl;
        this->collectorThread = std::thread([=] {
            int counter = 0;
            while (true) {
                void *tmpTask;
                //I have to check if the queue is empty or not.
                if (!inputQueue->isEmpty()) {
                    if ((tmpTask = inputQueue->safe_pop())) {
                        Task<T, U> *t = reinterpret_cast<Task<T, U> *>(tmpTask);

                        if (t->end == -1) {
                            counter++;
                            delete (t);
                            if (counter == this->activeWorkers) break;
                        } else {
                            std::chrono::duration<double> elapsed = t->endingTime - t->startingTime;
                            int TS = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count() / t->workingThreads;
                            int newNWorker;
                            if (*timeEmitter > *timeCollector) {
                                if (*timeEmitter > TS)
                                    newNWorker = round(float(*timeEmitter) / this->tsGoal);
                                else
                                    newNWorker = round(float(std::chrono::duration_cast<std::chrono::milliseconds>(t->endingTime - t->startingTime).count()) / this->tsGoal);
                            } else {
                                if (*timeCollector > TS)
                                    newNWorker = round(float(*timeCollector) / this->tsGoal);
                                else
                                    newNWorker = round(float(std::chrono::duration_cast<std::chrono::milliseconds>(t->endingTime - t->startingTime).count()) / this->tsGoal);
                            }

                            sendFeedback(newNWorker);

                            accumulator.push_back(t->result);
                            delete (t);
                        }
                    }
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