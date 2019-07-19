// @Authors Corbucci Luca
// clang-format off
#include <condition_variable>
#include <exception>
#include <iostream>
#include <limits>
#include <mutex>
#include <queue>
#include <thread>
// clang-format on

struct MyException : public std::exception {
    const char* what() const throw() {
        return "C++ Exception";
    }
};

template <class T>
class SafeQueue {
   private:
    std::mutex* d_mutex;
    std::condition_variable* p_condition;  //producer
    std::condition_variable* c_condition;  //consumer
    std::queue<T> queue;                   //initialize a local queue
    int max_size;
    bool ended = false;

   public:
    SafeQueue(int my_size) : max_size(my_size){};
    SafeQueue() {
        this->d_mutex = new std::mutex();
        p_condition = new std::condition_variable();
        c_condition = new std::condition_variable();
        this->max_size = std::numeric_limits<unsigned int>::max();
    };

    void safe_push(T item) {  //max size
        std::unique_lock<std::mutex> lock(*d_mutex);
        p_condition->wait(lock, [=] { return this->queue.size() < this->max_size; });  //if holds, it goes through

        this->queue.push(item);
        c_condition->notify_one();
        //wake a consumer since 1 element has been pushed in
    }

    bool safe_push_try(T item) {
        if (d_mutex->try_lock()) {
            this->queue.push(item);
            c_condition->notify_one();
            return true;
        }
        return false;
    }

    int safe_size() {
        std::lock_guard<std::mutex> lock(*d_mutex);
        return this->queue.size();
    }

    T safe_pop() {
        std::unique_lock<std::mutex> lock(*d_mutex);
        c_condition->wait(lock, [=] { return !this->queue.empty(); });  //if holds, it goes through

        T popped = this->queue.front();
        this->queue.pop();
        p_condition->notify_one();  //wake a producer since 1 element has been popped from
        return popped;
    }

    void empty_and_print() {
        while (!this->queue.empty()) {
            T popped = this->queue.front();
            this->queue.pop();
            std::cout << popped << std::endl;
        }
    }

    bool isEmpty() {
        std::unique_lock<std::mutex> lock(*d_mutex);
        return this->queue.empty();
    }
};