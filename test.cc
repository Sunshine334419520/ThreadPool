#include "thread_pool.h"

#include <stdio.h>
#include <memory>

class ThreadPoolTest {
public:
    ThreadPoolTest()
        : thread_pool_(new base::ThreadPool(4)) {
    }

    void TestBaseTask() {
        thread_pool_->Start();
        for (int i = 0; i < 30; i++) {
            thread_pool_->AddWork(std::bind(&ThreadPoolTest::Print, this));
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(2000));

        thread_pool_->AddWork(std::bind(&ThreadPoolTest::Print, this));

        thread_pool_->JoinAll();
    }

    void TestBaseTaskForReturn() {
        thread_pool_->Start();
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
        std::future<int> res[10];
        for (int i = 0; i < 10; i++) {
            res[i] = thread_pool_->AddWork(std::bind(&ThreadPoolTest::Add, this, 10, 20));
        }

        //  等待结果
        for (int i = 0; i < 10; i++) {
            printf("result i = %d\n", res[i].get());
        }

        thread_pool_->JoinAll();
    }
protected:
    void Print() {
        for (int i = 0; i < 10; ++i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            printf("current thread id = %ld, i = %d\n", std::this_thread::get_id(), i);
        }
    }

    int Add(int x, int y) {
        return x + y;
    }
    std::unique_ptr<base::ThreadPool> thread_pool_;
};

int main(void) {
    ThreadPoolTest thread_pool;
    thread_pool.TestBaseTask();
    thread_pool.TestBaseTaskForReturn();
    return 0;
}