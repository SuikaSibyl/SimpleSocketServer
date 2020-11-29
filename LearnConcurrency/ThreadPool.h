#pragma once

#include <iostream>
#include <thread>
#include <stdio.h>
#include <winsock2.h>
#include <vector>
#include <functional>
#include <queue>
#include <mutex>
#include <memory>
#include <atomic>

template<class T>
class thread_safe_queue
{
private:
	mutable std::mutex mut;
	std::queue<std::shared_ptr<T>> data_queue;
	std::condition_variable data_con;
public:
	thread_safe_queue() {}
	thread_safe_queue(thread_safe_queue const& other)
	{
		std::lock_guard<std::mutex> lk(other.mut);
		data_queue = other.data_queue;
	}
	void push(T tValue)
	{
		// Push one T into queue, and notify a waiting pop instruction
		std::shared_ptr<T> data(std::make_shared<T>(std::move(tValue)));
		std::lock_guard<std::mutex> lk(mut);
		data_queue.push(data);
		data_con.notify_one();
	}
	void wait_and_pop(T& tValue)
	{
		// Wait if mutex is locked or queue is empty
		// Else return the front as tValue, and pop it
		std::unique_lock<std::mutex> lk(mut);
		data_con.wait(lk, [this] {return !data_queue.empty(); });
		tValue = std::move(*data_queue.front());
		data_queue.pop();
	}
	std::shared_ptr<T>wait_and_pop()
	{
		// Wait if mutex is locked or queue is empty
		// Else return the front as shared_ptr, and pop it
		std::unique_lock<std::mutex> lk(mut);
		data_con.wait(lk, [this] {return !data_queue.empty(); });
		std::shared_ptr<T> ret(std::make_shared<T>(data_queue.front()));
		data_queue.pop();
		return ret;
	}
	bool try_pop(T& tValue)
	{
		// If queue is empty, then return false;
		// Else pop data_queue and return true.
		std::lock_guard<std::mutex> lk(mut);
		if (data_queue.empty())
			return false;
		tValue = std::move(*data_queue.front());
		data_queue.pop();
		return true;
	}

	std::shared_ptr<T> try_pop()
	{
		// If queue is empty, then return shared_ptr point to new T;
		// Else pop data_queue and return shared_ptr point to front obj.
		std::lock_guard<std::mutex> lk(mut);
		if (data_queue.empty())
			return std::shared_ptr<T>();
		std::shared_ptr<T> ret(std::make_shared(data_queue.front()));
		data_queue.pop();
		return ret;
	}

	bool empty() const
	{
		// Empty the queue
		std::lock_guard<std::mutex> lk(mut);
		return data_queue.empty();
	}
};

class join_threads
{
	std::vector<std::thread>& threads;
public:
	explicit join_threads(std::vector<std::thread>& threads_) :threads(threads_) {}
	~join_threads()
	{
		for (unsigned long i = 0; i < threads.size(); ++i)
		{
			if (threads[i].joinable())
				threads[i].join();
		}
	}
};

class ThreadPool
{
	std::atomic<bool> done;
	thread_safe_queue<std::function<void()>> work_queue;
	std::vector<std::thread> threads;
	join_threads jointer;

	void worker_thread()
	{
		while (!done)
		{
			std::function<void()> task;
			if (work_queue.try_pop(task))
			{
				task();
			}
			else
			{
				std::this_thread::yield();
			}
		}
	}

public:
	ThreadPool():done(false), jointer(threads)
	{
		unsigned const thread_count = std::thread::hardware_concurrency();

		try
		{
			for (unsigned i = 0; i < thread_count; i++)
			{
				threads.push_back(std::thread(&ThreadPool::worker_thread, this));
			}
		}
		catch (...)
		{
			done = true;
			throw;
		}
	}

	~ThreadPool()
	{
		done = true;
	}

	template<typename FunctionType>
	void submit(FunctionType f)
	{
		work_queue.push(std::function<void()>(f));
	}
};