#pragma once
#ifndef FEEDBACK_HPP
#define FEEDBACK_HPP
// clang-format off
#include <chrono>
// clang-format on

/// @brief Feedback structure
struct Feedback {
    /// @brief Number of active workers that we want to have in the next iteration
    int newNumberOfWorkers = -1;
    int senderID;
};

///  @brief Compute the new number of worker
///  @param int newNWorker
///  @param int currentWorkers
///  @param int count
///  @param int x
///  @param int maxWorkers
///  @return int The new number of workers
int createFeedback(int newNWorker, int &currentWorkers, int &count, int &x, int maxWorkers) {
    int n = -1;

    if (newNWorker != currentWorkers) {
        if (count == 0) {
            x = newNWorker;
            count = 1;
        } else {
            newNWorker == x ? count++ : count = 0;
        }

        if (count == 2) {
            currentWorkers = newNWorker;
            if (newNWorker == 0) {
                n = 1;
            } else if (newNWorker > maxWorkers) {
                n = maxWorkers;
            } else {
                n = newNWorker;
            }
            count = 0;
        }
    }
    return n;
}

#endif
