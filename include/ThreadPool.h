#include "BoundedQueue.h"
#include <functional>
#cinlude <vector>

template <typename T>

class ThreadPool{
    private:
        BoundedQueue<std::fucntion<void()>> taskQueue; // taskqueue must be written above workers to show dependacy of workers on taskqueue
        std::vector<std::thread> workers;

        void workerLoop(){
            
        }
    
    public:
        ThreadPool(size_t x): taskQueue{x}{}
        ThreadPool(size_t workerCount) {
            for (size_t i = 0; i < workerCount; i ++){
                workers.emplace_back(&ThreadPool::workerLoop, this);
            }
        }
};
