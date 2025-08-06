#include "ThreadPool.hpp"

ThreadPool::ThreadPool(size_t thread_count) {
    for (size_t i = 0; i < thread_count; ++i) {
        workers.emplace_back([this] {
            while (true) {
                std::function<void()> task;
                {
                    std::unique_lock lock(m);
                    cv.wait(lock, [&]{ return stop || !tasks.empty(); });
                    if (stop && tasks.empty()) return;
                    task = std::move(tasks.front());
                    tasks.pop();
                }
                task();
            }
        });
    }
}

ThreadPool::~ThreadPool() {
    {
        std::lock_guard lock(m);
        stop = true;
    }
    cv.notify_all();
    for (auto& w : workers) w.join();
}

void ThreadPool::enqueue(const std::function<void()>& f) {
    {
        std::lock_guard lock(m);
        tasks.emplace(std::move(f));
    }
    cv.notify_one();
}