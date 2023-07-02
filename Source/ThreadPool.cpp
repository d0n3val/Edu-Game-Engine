#include "Globals.h"

#include "ThreadPool.h"

#include <chrono>

ThreadPool::ThreadPool()
{
    uint numThreads = uint(std::thread::hardware_concurrency());
    workers.reserve(numThreads);

    for (uint i = 0; i < numThreads; ++i)
    {
        workers.push_back(std::thread(std::bind(&ThreadPool::WorkerFunc, this, i)));
    }
}

ThreadPool::~ThreadPool() 
{
    finish = true;
    cond.notify_all();

    for (std::thread& th : workers)
        th.join();
}

std::future<void> ThreadPool::submitTask(const std::function<void()>& function)
{
    std::packaged_task<void()> task(function);
    std::future<void> res = task.get_future();
    pushTask(task);
    cond.notify_one();

    return res;
}

void ThreadPool::WorkerFunc(uint id)
{
    while (!finish)
    {
        std::unique_lock<std::mutex> lock(condMutex);
        cond.wait(lock);
        lock.unlock();

        std::packaged_task<void()> func;
        while(popTask(func))
        {
            func();
        }
    }
}

bool ThreadPool::popTask(std::packaged_task<void()> &task)
{
    std::lock_guard<std::mutex> lock(taskMutex);
    if (!tasks.empty())
    {
        task = std::move(tasks.front());
        tasks.pop();

        return true;
    }

    return false;
}

void ThreadPool::pushTask(std::packaged_task<void()> &task)
{
    std::lock_guard<std::mutex> lock(taskMutex);
    tasks.push(std::move(task));
}