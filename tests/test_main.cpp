#include <gtest/gtest.h>
#include <atomic>
#include <thread>
#include <algorithm>
#include "BoundedQueue.h"   // adjust path to wherever the class lives
#include "ThreadPool.h"

TEST(BoundedQueue, PushThenPopReturnsSameValue) {
    BoundedQueue<int> q(4);
    ASSERT_TRUE(q.push(42));

    auto item = q.pop();
    ASSERT_TRUE(item.has_value());
    EXPECT_EQ(item.value(), 42);
}

TEST(BoundedQueue, SizeReflectsPushes) {
    BoundedQueue<int> q(4);
    EXPECT_EQ(q.size(), 0u);

    q.push(1);
    q.push(2);
    EXPECT_EQ(q.size(), 2u);
}

TEST(BoundedQueue, FifoOrdering) {
    BoundedQueue<int> q(4);
    q.push(1);
    q.push(2);
    q.push(3);

    EXPECT_EQ(q.pop().value(), 1);
    EXPECT_EQ(q.pop().value(), 2);
    EXPECT_EQ(q.pop().value(), 3);
}

TEST(BoundedQueue, PushAfterShutdownIsRejected) {
    BoundedQueue<int> q(4);
    q.shutdown();
    EXPECT_FALSE(q.push(42));
}

TEST(BoundedQueue, PopOnEmptyShutdownQueueReturnsNullopt) {
    BoundedQueue<int> q(4);
    q.shutdown();
    EXPECT_FALSE(q.pop().has_value());
}

TEST(ThreadPool, ExecuteAllSubmittedTasks){
    const int total_tasks = 100;
    std::atomic<int> counter{0};
    {
        ThreadPool pool(4);
        for (int i = 0; i < total_tasks; ++i) {
            bool accepted = pool.submit([&counter]() {
                counter.fetch_add(1, std::memory_order_relaxed);
            });
            EXPECT_TRUE(accepted);
        }
    }  
    EXPECT_EQ(counter.load(), total_tasks);
}

TEST(BoundedQueue, StressMultiProducerMultiConsumer) {
    const int kProducers = 4;
    const int kConsumers = 4;
    const int kPerProducer = 25000;

    BoundedQueue<int> q(64);  

    std::vector<std::thread> producers;
    std::vector<std::thread> consumers;
    std::vector<std::vector<int>> consumed(kConsumers);  

    for (int i = 0; i < kConsumers; i++){
        consumers.emplace_back([&, i](){
            while (auto item = q.pop()) {
                consumed[i].push_back(*item);
            }
        });
    }

    for (int p = 0; p < kProducers; p++){
        producers.emplace_back([&q, p](){
            for (int i = 0; i < kPerProducer; i++){
                q.push(p * kPerProducer + i);
            }
        });
    }

    for (auto& t : producers) {
        if (t.joinable()) {
            t.join();
        }
    }

    q.shutdown();

    for (auto& t : consumers) {
        if (t.joinable()) {
            t.join();
        }
    }
    
    std::vector<int> all;
    for (const auto& bucket : consumed) {
        all.insert(all.end(), bucket.begin(), bucket.end());
    }
    std::sort(all.begin(), all.end());

    ASSERT_EQ(all.size(), static_cast<size_t>(kProducers * kPerProducer));
    for (size_t i = 0; i < all.size(); i++) {
        ASSERT_EQ(all[i], static_cast<int>(i));
    }
}