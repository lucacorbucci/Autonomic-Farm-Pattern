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

// clang-format on
using namespace ff;

///  @brief Implementation of the Emitter of the autonomic farm
///  @detail Typename T is used as output type of the function that
///  the worker will compute. Typename U as output type of the function
///  that the worker will compute.
template <class T, class U>
class Emitter {
   private:
    ///  @brief Vector of outputQueue for the emitter.
    ///  I have an outputQueue for each of the workers
    std::vector<SWSR_Ptr_Buffer *> outputQueue;
    ///  @brief Emitter's thread
    std::thread emitterThread;
    ///  @brief Vector with all the workers
    std::vector<Worker<T, U> *> workerQueue;
    ///  @brief Feedback queue (worker to emitter)
    uSWSR_Ptr_Buffer *feedbackQueue;
    ///  @brief bit vector with the status (working/not working) of the workers
    //   activeWorkers:   0 -> working
    //                    1 -> available
    std::vector<int> activeWorkers;
    ///  @brief bit vector with the status (sleeping/not sleeping) of the workers
    //   sleepingWorkers: 0 -> active
    //                    1 -> sleeping
    std::vector<int> sleepingWorkers;
    ///  @brief Vector with the tasks to be computed
    std::vector<Task<T, U> *> inputVector;
    ///  @brief Max number of workers
    int maxnWorker;
    ///  @brief Number of sleeping workers
    int sleeping = 0;
    ///  @brief ID of the last worker that received a task
    int lastWorker = 0;
    ///  @brief Number of workers that are working
    int currentNumWorker;
    ///  @brief Number of workers that worked in the previous iteration
    int prevNumWorker;
    int x;
    int count = 0;
    uMPMC_Ptr_Queue *feedbackQueueWorker;
    std::atomic<int> *timeEmitter;
    std::chrono::high_resolution_clock::time_point lastUpdate;
    int stopTime;
    bool first = true;

    ///  @brief This function return the index of first active worker
    ///  @return The index of the first active worker
    int getFirstActive() {
        for (unsigned int i = this->lastWorker; i < activeWorkers.size(); i++) {
            if (activeWorkers[i] == 1 && sleepingWorkers[i] == 0) {
                this->lastWorker = i;
                return i;
            }
        }

        for (int i = 0; i < this->lastWorker; i++) {
            if (activeWorkers[i] == 1 && sleepingWorkers[i] == 0) {
                this->lastWorker = i;
                return i;
            }
        }
        return -1;
    }

    ///  @brief Send a sleep signal to a thread
    ///  @param int the ID of the thread to whom we want to send a sleep signal
    ///  @return Void
    void setSleeping(int index) {
        workerQueue[index]->stopWorker();
        sleepingWorkers[index] = 1;
        sleeping++;
    }

    ///  @brief Wake up a sleeping worker
    ///  @param int the ID of the thread to whom we want to send a wake up signal
    ///  @return Void
    void wakeUpWorker(int index) {
        workerQueue[index]->restartWorker();
        sleepingWorkers[index] = 0;
        sleeping--;
    }

    ///  @brief This function wake up the correct number of workers
    ///  @return Void
    void checkWakeUp() {
        unsigned int index = 0;
        while (sleeping + this->currentNumWorker != this->maxnWorker && index < sleepingWorkers.size()) {
            if (sleepingWorkers[index] == 1) {
                wakeUpWorker(index);
                lastUpdate = std::chrono::high_resolution_clock::now();
                first = false;
            }
            index++;
        }
    }
    ///  @brief This function send a wait signal to the correct number of workers
    ///  @return Void
    void checkSleep() {
        int index = 0;
        while (sleeping + this->currentNumWorker < this->maxnWorker && index < activeWorkers.size()) {
            if (sleepingWorkers[index] == 0) {  // && activeWorkers[index] == 1) {
                setSleeping(index);
                lastUpdate = std::chrono::high_resolution_clock::now();
                first = false;
            }
            index++;
        }
    }

    ///  @brief Wake up all the workers and then send a task
    ///  with a special value to terminate all the workers.
    void terminate() {
        unsigned int sent = 0;

        for (int i = 0; i < this->maxnWorker; i++) {
            workerQueue[i]->restartWorker();
            sleepingWorkers[i] = 0;
        }
        // I have to send the final task with value -1 to stop the workers
        while (sent < outputQueue.size()) {
            Task<T, U> *task = new Task<T, U>();
            task->end = -1;
            if (outputQueue[sent]->push(task)) {
                sent++;
            } else {
                delete (task);
            }
        }
    }

    ///  @brief This function takes the received feedback and extract the
    ///  informations about the new number of worker.
    ///  @param Feedback   The feedback from which we have to extract the information
    ///  @return Void
    void readFeedback(Feedback *f) {
        std::chrono::duration<double> elapsed = std::chrono::high_resolution_clock::now() - lastUpdate;
        int elapsedINT = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
        if (elapsedINT > stopTime || first) {
            if (f->newNumberOfWorkers != currentNumWorker && f->newNumberOfWorkers > 0) {
                this->prevNumWorker = this->currentNumWorker;
                this->currentNumWorker = f->newNumberOfWorkers;
                this->currentNumWorker < this->prevNumWorker
                    ? checkSleep()
                    : checkWakeUp();
            }
        }
    }

    ///  @brief This function extract the feedback from the feedback queue
    void receiveFeedback() {
        if (!this->feedbackQueue->empty()) {
            void *tmpF;
            bool res = this->feedbackQueue->pop(&tmpF);
            if (res) {
                Feedback *f = reinterpret_cast<Feedback *>(tmpF);
                readFeedback(f);
            }
        }

        void *tmpF;
        bool res = this->feedbackQueueWorker->pop(&tmpF);
        if (res) {
            Feedback *f = reinterpret_cast<Feedback *>(tmpF);
            activeWorkers[f->senderID] = 1;
        }
    }

    ///  @brief code of the emitter of the autonomic farm
    ///  @details
    ///  This function extracts a task from the input vector and push this task
    ///  to the input spsc queue of an active worker.
    ///  @return Void
    void threadBody() {
        unsigned int i = 0;
        int index = 0;
        std::chrono::high_resolution_clock::time_point start;
        std::chrono::high_resolution_clock::time_point end;
        lastUpdate = std::chrono::high_resolution_clock::now();

        while (i < inputVector.size()) {
            start = std::chrono::high_resolution_clock::now();
            receiveFeedback();

            index = getFirstActive();
            if (index >= 0) {
                Task<T, U> *task = inputVector.at(i);
                task->workingThreads = this->currentNumWorker;

                if (outputQueue[index]->push(task)) {
                    activeWorkers[index] = 0;
                    i++;
                    end = std::chrono::high_resolution_clock::now();
                    std::chrono::duration<double> elapsed = end - start;
                    *timeEmitter = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
                }
            }
        }

        terminate();
    }

   public:
    ///  @brief Constructor method of the Emitter component
    ///  @param *outputQueue        The queue where the emitter send a new task
    ///  @param workerQueue         The workers's queue
    ///  @param nWorker             The initial number of active workers
    ///  @param FeedbackQueue       The feedback queue used to send back a feedback to the emitter
    ///  @param inputVector         Input Vector with the tasks that has to be computed
    ///  @param FeedbackQueueWorker The feedback queue used to send back a feedback to the emitter from the worker
    ///  @param timeEmitter         Emitter's service time
    ///  @param time                This is time that we have to wait to change the number of workers of the farm
    Emitter(std::vector<SWSR_Ptr_Buffer *> outputQueue, std::vector<Worker<T, U> *> workerQueue, int nWorker, uSWSR_Ptr_Buffer *feedbackQueue, std::vector<Task<T, U> *> inputVector, uMPMC_Ptr_Queue *feedbackQueueWorker, std::atomic<int> *timeEmitter, int time) {
        this->outputQueue = outputQueue;
        this->workerQueue = workerQueue;
        this->maxnWorker = nWorker;
        this->feedbackQueue = feedbackQueue;
        this->inputVector = inputVector;
        activeWorkers.reserve(nWorker);
        sleepingWorkers.reserve(nWorker);
        currentNumWorker = nWorker;
        prevNumWorker = nWorker;
        x = nWorker;
        this->feedbackQueueWorker = feedbackQueueWorker;
        for (int i = 0; i < nWorker; i++) {
            activeWorkers.push_back(1);
        }
        for (int i = 0; i < nWorker; i++) {
            sleepingWorkers.push_back(0);
        }
        this->timeEmitter = timeEmitter;
        this->stopTime = time;
    }

    ///  @brief Start the thread of the emitter componentend.
    ///  @return Void
    void start() {
        this->emitterThread = std::thread([=] {
            threadBody();
        });
    }

    ///  @brief This function is used to join the emitter thread
    ///  @return Void
    void join() {
        this->emitterThread.join();
    }
};