#pragma once

#include <thread>
#include <condition_variable>
#include <mutex>
#include <functional>
#include <queue>
#include <future>

class ThreadPool
{
    bool finish = false;
	std::vector<std::thread> workers;
    std::queue < std::packaged_task<void()> > tasks;

    std::mutex taskMutex;
    std::mutex condMutex;
    std::condition_variable cond;

public:    
    ThreadPool();
    ~ThreadPool();
    std::future<void> submitTask(const std::function<void()>& function);

private:

    void WorkerFunc(uint id);
    bool popTask(std::packaged_task<void()>& task);
    void pushTask(std::packaged_task<void()>& task);
};
