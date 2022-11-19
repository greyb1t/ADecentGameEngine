#include "ThreadPool.h"

ThreadPool::ThreadPool(const int aThreadCount)
	: myThreadCount(aThreadCount)
{
	for (int i = 0; i < aThreadCount; ++i)
	{
		myThreads.emplace_back(&ThreadPool::LookingForTasksThread, this);
	}
}

ThreadPool::~ThreadPool()
{
	StopJoinAll();
}

int ThreadPool::GetThreadCount() const
{
	return myThreadCount;
}

void ThreadPool::StopJoinAll()
{
	// do not stop if already stopped
	if (myShouldExit)
	{
		return;
	}

	myShouldExit = true;

	myConditionVariable.notify_all();

	for (auto& thread : myThreads)
	{
		if (thread.joinable())
		{
			thread.join();
		}
	}
}

std::vector<std::thread::id> ThreadPool::GetThreadIds() const
{
	std::vector<std::thread::id> result;

	for (const auto& thread : myThreads)
	{
		result.push_back(thread.get_id());
	}

	return result;
}

void ThreadPool::WaitForTasks()
{
	std::unique_lock lock(myMutex);
	myConditionVariableFinished.wait(lock,
		[this]()
		{
			return myTasks.empty() && totalWorkersCurrentWorking == 0;
		});
}

void ThreadPool::LookingForTasksThread()
{
	while (true)
	{
		{
			std::function<void()> task;

			std::unique_lock<std::mutex> lock(myMutex);
			myConditionVariable.wait(lock, [this]()
				{
					if (myShouldExit)
					{
						return true;
					}
					else if (!myTasks.empty())
					{
						return true;
					}

					// Continue waiting
					return false;
				});

			if (myShouldExit == false)
			{
				++totalWorkersCurrentWorking;

				task = std::move(myTasks.front());
				myTasks.pop();
			}

			if (myShouldExit)
			{
				break;
			}

			lock.unlock();

			task();

			lock.lock();

			--totalWorkersCurrentWorking;

			myConditionVariableFinished.notify_all();
		}
	}
}
