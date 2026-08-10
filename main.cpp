#include <iostream>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <optional>

template <typename T>

class BoundedQueue {
    private:
        std::condition_variable not_full;
        std::condition_variable not_empty;
        mutable std::mutex mtx; // made mutable so const size method can use
        std::queue<T> container;
        const size_t capacity;
        bool stopping = false;

    public:
        BoundedQueue(size_t x): capacity{x}{}
        ~BoundedQueue(){shutdown();}
        bool push(const T& insert){
            std::unique_lock<std::mutex> lock(mtx);

            not_full.wait(lock, [this]{ return (container.size() < capacity || stopping);});    
            if (stopping){
                return false;
            }
            container.push(insert);
            not_empty.notify_one();
            return true;
        }

        std::optional<T> pop(){
            std::unique_lock<std::mutex> lock(mtx);

            not_empty.wait(lock, [this]{ return (container.size() > 0 || stopping);});

            if(container.size() > 0){
                T curr = std::move(container.front());
                container.pop();
                not_full.notify_one();
                return curr;
            }
            return std::nullopt;
        }
        void shutdown(){
            std::unique_lock<std::mutex> lock(mtx);
            if (stopping == true){
                return;
            }

            stopping = true;
            lock.unlock(); // unlocking before notifying avoids woken thread block on held lock
            not_full.notify_all();
            not_empty.notify_all();
        }
        size_t size() const{
            std::unique_lock<std::mutex> lock(mtx);
            return container.size();
        }
        
};


int main() {
    BoundedQueue<int> queue(3);

    std::cout << "Initializing Durable Task Engine (C++17)..." << std::endl;
    bool thread_finished = false;
    int curr;

    std::thread t([&thread_finished, &queue]() {
        queue.push(123);
        queue.push(123);
        queue.push(123);
        //thread_finished = true;
    });

    std::thread t2([&thread_finished, &queue, &curr]() {
        curr = queue.pop().value();
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