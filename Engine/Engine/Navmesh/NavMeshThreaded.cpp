#include "pch.h"
#include "NavMeshThreaded.h"

#include "NavMesh.h"
#include "Engine\Renderer\TracyProfilingGPU.h"

NavMeshThreaded::~NavMeshThreaded()
{
	myThreadRunning = false;
	myThread.join();
}

void NavMeshThreaded::Init()
{
	myThreadRunning = true;
	myThread = std::thread([this]() { ThreadedUpdate(); });
}

void NavMeshThreaded::SetNavMesh(NavMesh::NavMesh* aNavMesh)
{
	// THIS IS NOT THREAD SAFE
	myNavMesh = aNavMesh;
}

// Might want to add priority support to make player more responsive if needed
int NavMeshThreaded::StartJob(const CU::Vector3f& aStartPos, const CU::Vector3f& aEndPos)
{
	//LOG_INFO(LogType::Jesper) << "Start job" << myID;

	int id = ++myID;
	myID %= myIDSize;

	NavMeshJob job;
	job.id = id;
	job.startPos = aStartPos;
	job.endPos = aEndPos;

	myAddLock.lock();
	myJobsToAdd.emplace_back(job);
	myAddLock.unlock();

	return id;
}

void NavMeshThreaded::CancelJob(int aID)
{
	myAddLock.lock();
	auto it = std::find(myJobsToAdd.begin(), myJobsToAdd.end(), aID);
	if (it != myJobsToAdd.end())
	{
		//LOG_INFO(LogType::Jesper) << "Cancel job" << myID;
		myJobsToAdd.erase(it);
		myAddLock.unlock();
		return;
	}

	
	myJobsToCancel.emplace_back(aID);
	myAddLock.unlock();
}

//bool NavMeshThreaded::DoJobExist(int aID)
//{
//	myFinishedLock.lock();
//	const auto result = std::find(myJobsFinished.begin(), myJobsFinished.end(), aID) != myJobsFinished.end();
//	myFinishedLock.unlock();
//	return result;
//}

bool NavMeshThreaded::IsJobDone(int aID)
{
	/*for (size_t i = 0; i < myJobs.size(); i++)
	{
		LOG_INFO(LogType::Jesper) << "myJobs: " << myJobs[i].id;
	}*/

	// Might be too slow could be changed 
	myFinishedLock.lock();
	const auto result = std::find(myJobsFinished.begin(), myJobsFinished.end(), aID) != myJobsFinished.end();
	myFinishedLock.unlock();
	return result;
}

NavMeshThreaded::NavMeshJob NavMeshThreaded::BlowJob(int aID)
{
	std::lock_guard<std::mutex> guard(myFinishedLock);
	auto it = std::find(myJobsFinished.begin(), myJobsFinished.end(), aID);
	if (it == myJobsFinished.end())
	{
		return NavMeshJob();
	}

	NavMeshJob job = *it;
	myJobsFinished.erase(it);
	return job;
}

void NavMeshThreaded::ThreadedUpdate()
{
	while (myThreadRunning)
	{
		myAddLock.lock();
		for (int i = 0; i < myJobsToAdd.size(); i++)
		{
			myJobs.emplace_back(myJobsToAdd[i]);
		}

		for (int i = 0; i < myJobsToCancel.size(); i++)
		{
			auto it = std::find(myJobs.begin(), myJobs.end(), myJobsToCancel[i]);
			if (it != myJobs.end())
			{
				myJobs.erase(it);
				// myAddLock.unlock();
				break;
			}
		}

		myJobsToAdd.clear();
		myJobsToCancel.clear();
		myAddLock.unlock();
		

		if (myJobs.empty())
			continue;

		auto begin = myJobs.begin();
		NavMeshJob job = *begin;
		myJobs.erase(begin);

		job.path = myNavMesh->Pathfind(job.startPos, job.endPos);

		myFinishedLock.lock();
		myJobsFinished.emplace_back(job);
		myFinishedLock.unlock();
	}
}

