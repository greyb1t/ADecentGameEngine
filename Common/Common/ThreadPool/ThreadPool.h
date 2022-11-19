#pragma once

#include <queue>
#include <condition_variable>
#include <thread>
#include <mutex>
#include <atomic>
#include <type_traits>
#include <future>
#include <assert.h>

class ThreadPool
{
	struct Range
	{
		int myStart;
		int myEnd;
	};

public:
	ThreadPool(const int aThreadCount);
	~ThreadPool();

	// Task with no arguments, no return value
	template <typename F>
	void PushTask(const F& aFunc);

	// Task with arguments, no return value
	// Example: tp.PushTask([](int test) { std::cout << test; }, 1337);
	template <typename F, typename ...Args>
	void PushTask(const F& aFunc, const Args&... aArgs);

	template <typename F, typename ...Args>
	std::future<typename std::invoke_result<F, Args...>::type> PushTaskFuture(
		const F& aFunc,
		const Args&... aArgs);

	template <typename F>
	void ParallellLoop(const int aIterationCount, const F& aFunc);

	int GetThreadCount() const;

	void StopJoinAll();

	std::vector<std::thread::id> GetThreadIds() const;

	void WaitForTasks();

private:
	ThreadPool(const ThreadPool&) = delete;

	void LookingForTasksThread();

	std::queue<std::function<void()>> myTasks;
	std::mutex myMutex;
	std::condition_variable myConditionVariable;
	std::condition_variable myConditionVariableFinished;
	std::vector<std::thread> myThreads;

	std::atomic_int totalWorkersCurrentWorking = 0;

	std::atomic_bool myShouldExit = false;

	const int myThreadCount;

	std::mutex myMutexParallell;
	std::condition_variable cond;

	std::vector<Range> myParallellRanges;
};

template <typename F>
void ThreadPool::PushTask(const F& aFunc)
{
	{
		std::lock_guard<std::mutex> lock(myMutex);
		myTasks.push(std::function<void()>(aFunc));
	}

	myConditionVariable.notify_one();
}

template <typename F, typename ...Args>
void ThreadPool::PushTask(const F& aFunc, const Args&... aArgs)
{
	PushTask([aFunc, aArgs...]()
	{
		aFunc(aArgs...);
	});
}

template <typename F, typename ...Args>
std::future<typename std::invoke_result<F, Args...>::type>
ThreadPool::PushTaskFuture(const F& aFunc, const Args&... aArgs)
{
	using ReturnType = typename std::invoke_result<F, Args...>::type;

	auto promise = std::make_shared<std::promise<ReturnType>>();

	auto future = promise->get_future();

	PushTask([promise, aFunc, aArgs...]()
	{
		promise->set_value(aFunc(aArgs...));
	});

	return future;
}

template <typename F>
void ThreadPool::ParallellLoop(const int aIterationCount, const F& aFunc)
{
	if (aIterationCount <= 0)
	{
		return;
	}

	int threadCount = myThreadCount;

	// If less iterations than threads, only use 
	// the max needed amount of threads
	if (aIterationCount < myThreadCount)
	{
		threadCount = aIterationCount;
	}

	std::atomic_int threadsRunning = 0;

	myParallellRanges.reserve(threadCount);
	myParallellRanges.clear();

	const int normalBatch = aIterationCount / threadCount;
	const int remainderCount = aIterationCount - (normalBatch * threadCount);

	Range range;

	if (remainderCount > 0)
	{
		range.myStart = 0;
		range.myEnd = normalBatch + 1;
	}
	else
	{
		range.myStart = 0;
		range.myEnd = normalBatch;
	}

	myParallellRanges.push_back(range);
	++threadsRunning;

	for (int i = 1; i < threadCount; ++i)
	{
		int batch = normalBatch;

		if (i < remainderCount)
		{
			batch += 1;
		}

		range.myStart = myParallellRanges[i - 1].myEnd;
		range.myEnd = range.myStart + batch;

		++threadsRunning;

		myParallellRanges.push_back(range);
	}

	// Start the tasks
	for (size_t i = 0; i < myParallellRanges.size(); ++i)
	{
		const int startIndex = myParallellRanges[i].myStart;
		const int endIndex = myParallellRanges[i].myEnd;

		PushTask([this, aFunc, startIndex, endIndex, &threadsRunning]()
			{
				aFunc(startIndex, endIndex);
				--threadsRunning;

				{
					std::unique_lock lock2(myMutexParallell);
					cond.notify_one();
				}
			});
	}

	std::unique_lock lock(myMutexParallell);
	cond.wait(lock,
		[&threadsRunning]()
		{
			return threadsRunning <= 0;
		});

	//WaitForTasks();

	// while (threadsRunning > 0)
	// {
	// 	std::this_thread::yield();
	// }
}