#include "BoundedQueue.h"
#include <functional>
#include <vector>
#include <iostream>
#include <thread>

class ThreadPool{
    private:
        BoundedQueue<std::function<void()>> taskQueue; // taskqueue must be written above workers to show dependacy of workers on taskqueue
        std::vector<std::thread> workers; // fixed # of workers/threads
        void workerLoop(){
            while (true) { // worker will keep looking for task until queue is shut down
                auto result = taskQueue.pop(); // pop function from boundedqueue
                if (!result) break; // no tasks
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
                workers.emplace_back(&ThreadPool::workerLoop, this); // creates a new thread and put into worker vector
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
        bool submit(std::function<void()> task){ // hands tasks to taskQueue and return boolean based on if accepted or rejected
            return taskQueue.push(std::move(task));
        }

};
