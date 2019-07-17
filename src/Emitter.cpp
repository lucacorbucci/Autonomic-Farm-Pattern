// clang-format off
#include <unistd.h>
#include <iostream>
#include <utility>
#include <vector>
// clang-format on

template <class T>
class Emitter {
   private:
    SafeQueue<T> *outputQueue;
    std::thread emitterThread;

   public:
    Emitter(SafeQueue<T> *outputQueue) {
        this->outputQueue = outputQueue;
    }

    void start() {
        std::cout << "Emitter avviato" << std::endl;
        this->emitterThread = std::thread([=] {
            int i = 10;
            while (i > 0) {
                outputQueue->safe_push(i);
                i--;
            }
            outputQueue->safe_push(-1);
        });

        this->emitterThread.join();
    }
};