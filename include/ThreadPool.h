#include "BoundedQueue.h"
#include <functional>
#include <vector>
#include <iostream>
#include <thread>

class ThreadPool{
    private:
        BoundedQueue<std::function<void()>> taskQueue; // taskqueue must be written above workers to show dependacy of workers on taskqueue
        std::vector<std::thread> workers;
        void workerLoop(){
            while (true) {
                auto result = taskQueue.pop();
                if (!result) break;
                try {
                    result.value()(); 
                } catch (const std::exception& e) {
                    std::cerr << "task threw: " << e.what() << std::endl; // standard catch describing what went wrong through e.what()
                } catch (...) { //catch anything that passes first catch
                    std::cerr << "task threw unknown exception" << std::endl;
                }
            }
        }
    
    public:
        ThreadPool(size_t workerCount) : taskQueue(1024){
            for (size_t i = 0; i < workerCount; i ++){
                workers.emplace_back(&ThreadPool::workerLoop, this);
            }
        }
        ~ThreadPool() {
            taskQueue.shutdown();
            for (std::thread& w : workers) {
                if (w.joinable()) {
                    w.join();
                }
            }
        }

};
