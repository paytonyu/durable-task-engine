#include <gtest/gtest.h>
#include <atomic>
#include <thread>
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