#pragma once
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>

class ThreadPool
{
public:
    ThreadPool(size_t thread_nums = std::thread::hardware_concurrency())
        :_thread_nums(thread_nums)
        ,_stop(false)
    {
        for (size_t i = 0; i < _thread_nums; ++i)
        {
          _vt.emplace_back(std::thread(&ThreadPool::LoopWork, this));
        }
    }

    //禁用拷贝构造和operator=
    ThreadPool(const ThreadPool &) = delete;
    ThreadPool& operator=(const ThreadPool &) = delete;

private:
    //线程的执行函数
    void LoopWork()
    {
        std::unique_lock<std::mutex> ul(_mtx);
        for (;;)
        {
            while(_taskQueue.empty())
            {
              _isEmpty.wait(ul);
            }

            if(_stop){
                break;
            }

            auto task = std::move(_taskQueue.front());
            _taskQueue.pop();
            ul.unlock();
            task();     //执行任务
            ul.lock();  //下一轮循环依然加锁
        }
    }

public:
    template<typename F>
    void PushTask(F&& task)
    {
        {
            std::lock_guard<std::mutex> lg(_mtx);
            _taskQueue.push(std::forward<F>(task)); //任务队列是临界资源(其他地方会修改)
        }
        _isEmpty.notify_one();  //条件变量的通知并不会因为多线程而影响结果(因此可以不加锁)
    }


    ~ThreadPool()
    {
        _stop = true;
        _isEmpty.notify_all();
        for (size_t i = 0; i < _thread_nums; ++i)
        {
            if(_vt[i].joinable()){
                _vt[i].join();
            }
        }
    }

private:
    size_t _thread_nums;
    std::vector<std::thread> _vt;
    std::queue<std::function<void()>> _taskQueue;
    std::mutex _mtx;
    std::condition_variable _isEmpty;
    std::atomic<bool> _stop;
};
