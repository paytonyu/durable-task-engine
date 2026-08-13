#include <iostream>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <optional>
#include <chrono>

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
    std::cout << "durable-task-engine" << std::endl;

    BoundedQueue<int> queue(3);
    return 0;
}