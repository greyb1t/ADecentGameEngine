#pragma once
#include <queue>
#include <thread>
#include <mutex>

namespace NavMesh
{
	class NavMesh;
}

class NavMeshThreaded
{
	friend class NavJobHandler;

	struct NavMeshJob
	{
		int id = -1;
		CU::Vector3f startPos;
		CU::Vector3f endPos;
		std::vector<CU::Vector3f> path;

		friend bool operator==(const NavMeshJob& a, const NavMeshJob& b)
		{
			return a.id == b.id;
		}

		friend bool operator==(const NavMeshJob& a, const int aID)
		{
			return a.id == aID;
		}

		friend bool operator==(const int aID, const NavMeshJob& a)
		{
			return a.id == aID;
		}
	};


public:
	~NavMeshThreaded();

	NavMeshThreaded() = default;
	NavMeshThreaded(const NavMeshThreaded&) = delete;

	void Init();

	void SetNavMesh(NavMesh::NavMesh* aNavMesh);

	int StartJob(const CU::Vector3f& aStartPos, const CU::Vector3f& aEndPos);
	void CancelJob(int aID);

	bool IsJobDone(int aID);
	NavMeshJob BlowJob(int aID);

private:
	void ThreadedUpdate();
private:
	NavMesh::NavMesh* myNavMesh = nullptr;

	const int myIDSize = 10000;
	int myID = 0;

	std::mutex myAddLock;
	std::mutex myFinishedLock;

	std::vector<NavMeshJob> myJobsToAdd;
	std::vector<int> myJobsToCancel;
	//std::queue<NavMeshJob> myJobs;
	std::vector<NavMeshJob> myJobs;
	std::vector<NavMeshJob> myJobsFinished;

	// Threading
	bool myThreadRunning = false;
	std::thread myThread;
};