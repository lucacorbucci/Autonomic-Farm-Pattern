/*
    author: Luca Corbucci
    student number: 516450
*/

// clang-format off
#include <unistd.h>
#include <iostream>
#include <utility>
#include <vector>
#include <ff/ubuffer.hpp>
#include <ff/mpmc/MPMCqueues.hpp>

// clang-format on
using namespace ff;

///  @brief Implementation of the collector of the autonomic farm
///  @detail Typename T is used for as output type of the function that
///  the worker will compute. Typename U is input as output type of the function
///  that the worker will compute.
template <class T, class U>
class Collector {
   private:
    uMPMC_Ptr_Queue *inputQueue;
    std::thread collectorThread;
    uSWSR_Ptr_Buffer *feedbackQueue;
    int activeWorkers;
    int tsGoal;
    int maxWorkers;
    int count = 0;
    int x;
    int currentWorkers;
    std::atomic<int> *timeEmitter;
    std::atomic<int> *timeCollector;

    ///  @brief Constructor method of the Collector component
    ///  @param *InputQueue   The queue from which we extract the computed tasks
    ///  @param activeWorkers The initial number of active workers
    ///  @param TsGoal        The expected service time
    ///  @param FeedbackQueue The feedback queue used to send back a feedback to the emitter
    ///  @param timeEmitter   Emitter's service time
    ///  @param timeCollector Collector's service time
   public:
    Collector(uMPMC_Ptr_Queue *inputQueue, int activeWorkers, int tsGoal, uSWSR_Ptr_Buffer *feedbackQueue, std::atomic<int> *timeEmitter, std::atomic<int> *timeCollector) {
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
    std::vector<Task<T, U> *> accumulator;

    ///  @brief This method is used to create and send the feedback from the collector to the emitter
    ///  @param newNWorker  New number of workers
    void sendFeedback(int newNWorker) {
        Feedback f;
        int n = createFeedback(newNWorker, currentWorkers, maxWorkers);
        if (n > 0)
            f.newNumberOfWorkers = n;

        this->feedbackQueue->push(&f);
    }

    ///  @brief Start the collector componentend.
    ///  @details
    ///  This function extracts a computed task from the input queue and computes
    ///  the number of workers that will be used to compute the next tasks
    ///  based on the elapsed time of the popped task.
    ///  This function uses a spsc queue to communicate with the emitter to change
    ///  the number of workers
    ///  @return Void
    void start() {
        this->collectorThread = std::thread([=] {
            int counter = 0;
            std::chrono::high_resolution_clock::time_point start;
            std::chrono::high_resolution_clock::time_point end;

            while (true) {
                start = std::chrono::high_resolution_clock::now();
                void *tmpTask;
                // Estraggo dalla coda
                if (inputQueue->pop(&tmpTask)) {
                    Task<T, U> *t = reinterpret_cast<Task<T, U> *>(tmpTask);
                    // Controllo se è il task che mi dice di terminare o no
                    if (t->end == -1) {
                        counter++;
                        delete (t);
                        if (counter == this->activeWorkers) break;
                    } else {
                        std::chrono::duration<double> elapsed = t->endingTime - t->startingTime;
                        int TS = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count() / t->workingThreads;
                        int newNWorker;
                        // Calcolo il massimo tra i vari tempi
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

                        // invio il feedback all'emitter
                        sendFeedback(newNWorker);
                        // memorizzo il risultato del task calcolato
                        accumulator.push_back(t);
                    }
                }
                end = std::chrono::high_resolution_clock::now();
                std::chrono::duration<double> elapsed = end - start;
                *timeCollector = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
            }
        });
    }

    ///  @brief This function is used to join the thread
    ///  @return Void
    void join() {
        this->collectorThread.join();
    }
};