#pragma once // protects compiler from reading it twice and duplicating class definition


// This class is a thread-safe queue with a maximum capacity,
// allowing multiple threads to safely push and pop without
// exceeding capacity or accessing the queue at the same time.\

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
        bool push(const T& insert){  // insert not modified
            std::unique_lock<std::mutex> lock(mtx); // in every method, stops multiple threads from accessing at same time

            not_full.wait(lock, [this]{ return (container.size() < capacity || stopping);}); // wait until queue has space OR queue is shutting down
            if (stopping){
                return false;
            }
            container.push(insert);
            not_empty.notify_one(); // notifies ONE thread that something has been pushed, if it is waiting to pop()
            return true;
        }

        std::optional<T> pop(){
            std::unique_lock<std::mutex> lock(mtx);

            not_empty.wait(lock, [this]{ return (container.size() > 0 || stopping);});

            if(container.size() > 0){
                T curr = std::move(container.front()); // std::move to 
                container.pop(); 
                not_full.notify_one(); //notify one thread that something has been popped
                return curr;
            }
            return std::nullopt; // no item
        }
        void shutdown(){
            std::unique_lock<std::mutex> lock(mtx);
            if (stopping == true){ // if already stopping just return
                return;
            }

            stopping = true;
            lock.unlock(); // unlocking before notifying avoids woken thread block on held lock
            not_full.notify_all(); // tell all threads the queue is shutting down
            not_empty.notify_all();
        }
        size_t size() const{
            std::unique_lock<std::mutex> lock(mtx);
            return container.size();
        }
        
};