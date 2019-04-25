#pragma once

#include <iostream>
#include <algorithm>
#include <memory>
#include <queue>
#include <vector>
#include <functional>

#include <thread>
#include <mutex>
#include <condition_variable>

using std::cout;
using std::function;
using std::queue;
using std::vector;

using std::thread;
using std::mutex;
using std::condition_variable;
using std::unique_lock;
using std::function;

using std::shared_ptr;
using std::make_shared;

class Worker
{
private:
    volatile bool running = true;
    volatile bool executing = false;

    thread* thread_;
    mutex mutex_;
    condition_variable condition_;
    condition_variable condition_join_;
    function<void()> job;

public:
    Worker(function<void()> job) :
        job(job)
    {
        thread_ = new thread([=] {
            thread_loop();
        });
    }

    ~Worker()
    {
        Terminate();
    }

    void Terminate()
    {
        if (running)
        {
            running = false;
            condition_.notify_one();
            thread_->join();
            delete thread_;
        }
    }

    void Notify()
    {
        executing = true;
        condition_.notify_one();
    }

    void Join()
    {
        if (executing)
        {
            unique_lock<mutex> lock(mutex_);
            condition_join_.wait(lock, [this] {
                return !executing || !running;
            });
            executing = false;
        }
    }

private:
    void thread_loop()
    {
        do
        {
            unique_lock<mutex> lock(mutex_);
            condition_.wait(lock, [this] {
                return executing || !running;
            });

            if (running && executing)
            {
                job();
            }

            condition_join_.notify_one();
            executing = false; //
        }
        while (running);
    }
};

class WorkerGroup
{
private:
    vector<shared_ptr<Worker>> workers_;

public:
    WorkerGroup()
    {
    }

    ~WorkerGroup()
    {
        Terminate();
    }

    void AddWorker(function<void()> job)
    {
        workers_.push_back(make_shared<Worker>(job));
    }

    void Notify()
    {
        for (auto w = workers_.begin(); w != workers_.end(); ++w)
        {
            (*w)->Notify();
        }
    }

    void Join()
    {
        for (auto w = workers_.begin(); w != workers_.end(); ++w)
        {
            (*w)->Join();
        }
    }

    void Run()
    {
        Notify();
        Join();
    }

    void Terminate()
    {
        for (auto w = workers_.begin(); w != workers_.end(); ++w)
        {
            (*w)->Terminate();
        }
    }
};
