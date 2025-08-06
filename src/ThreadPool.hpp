#pragma once
#include <queue>
#include <thread>
#include <vector>
#include <condition_variable>
#include <functional>

class ThreadPool {
public:
    ThreadPool(size_t thread_count = std::thread::hardware_concurrency());
    ~ThreadPool();
    void enqueue(const std::function<void()>& f);

private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex m;
    std::condition_variable cv;
    bool stop{false};
};