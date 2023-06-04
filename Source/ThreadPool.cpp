#include "Globals.h"

#include "ThreadPool.h"

#include <chrono>

ThreadPool::~ThreadPool()
{
    end();
}

void ThreadPool::init(uint numThreads)
{
    workers.reserve(numThreads);

    for (uint i = 0; i < numThreads; ++i)
    {
        workers.push_back(std::thread(std::bind(&ThreadPool::WorkerFunc, this, i)));
        workers.back().detach();
    }
}

void ThreadPool::end()
{
    finish = true;
    cond.notify_all();
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
    using namespace std::chrono_literals;

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