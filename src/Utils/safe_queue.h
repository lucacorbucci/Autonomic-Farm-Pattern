#pragma once
#ifndef SAFE_QUEUE_HPP
#define SAFE_QUEUE_HPP
#include <limits>
#include <queue>

template <class T>
class SafeQueue {
   private:
    std::mutex d_mutex;
    std::condition_variable p_condition;
    std::condition_variable c_condition;
    std::queue<T> queue;
    int max_size;

   public:
    SafeQueue(int my_size) : max_size(my_size){};
    SafeQueue() : max_size(std::numeric_limits<unsigned int>::max()){};

    void safe_push(T item) {
        std::unique_lock<std::mutex> lock(d_mutex);
        this->p_condition.wait(lock, [=] { return this->queue.size() < this->max_size; });
        this->queue.push(item);
        c_condition.notify_all();
    }

    bool safe_push_try(T item) {
        if (d_mutex.try_lock()) {
            this->queue.push(item);
            c_condition.notify_all();
            return true;
        }
        return false;
    }

    int safe_size() {
        std::lock_guard<std::mutex> lock(d_mutex);
        return this->queue.size();
    }

    T safe_pop() {
        std::unique_lock<std::mutex> lock(d_mutex);
        this->c_condition.wait(lock, [=] { return !this->queue.empty(); });
        T popped = this->queue.front();
        this->queue.pop();
        p_condition.notify_all();
        return popped;
    }

    void empty_and_print() {
        while (!this->queue.empty()) {
            T popped = this->queue.front();
            this->queue.pop();
        }
    }

    bool isEmpty() {
        std::unique_lock<std::mutex> lock(d_mutex);
        return this->queue.empty();
    }
};

#endif