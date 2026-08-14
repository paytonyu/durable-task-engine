#include <iostream>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <optional>
#include <chrono>
#include "BoundedQueue.h"
#include "ThreadPool.h"

int main() {
    std::cout << "durable-task-engine" << std::endl;

    BoundedQueue<int> queue(3);
    return 0;
}