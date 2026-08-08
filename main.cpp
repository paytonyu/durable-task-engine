#include <iostream>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>

template <typename T>

class BoundedQueue {
    private:
        std::condition_variable not_full;
        std::condition_variable not_empty;
        std::mutex mtx;
        std::queue<T> container;
        const size_t capacity;


    public:
        BoundedQueue(size_t x): capacity{x}{}
        void push(const T& insert){
            std::unique_lock<std::mutex> lock(mtx);

            not_full.wait(lock, [this]{ return container.size() < capacity;});    
            
            container.push(insert);

            not_empty.notify_one();
        }


        std::optional<T> pop(){

        }
        void shutdown(){

        }
        size_t size() const{

        }
        
};


int main() {
    BoundedQueue<int> queue(3);

    std::cout << "Initializing Durable Task Engine (C++17)..." << std::endl;
    bool thread_finished = false;

    std::thread t([&thread_finished, &queue]() {
        queue.push(123);
        queue.push(123);
        queue.push(123);
    });

    std::thread t2([&thread_finished, &queue]() {
        queue.push(123);
        thread_finished = true;
    });

    std::this_thread::sleep_for(std::chrono::seconds(1));

    if (thread_finished == true) {
        std::cout << "push works" << std::endl;
    } else {
        std::cout << "push blocked" << std::endl;
    }

    t.join();
    return 0;
}