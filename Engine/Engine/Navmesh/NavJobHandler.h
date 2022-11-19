#pragma once
#include "Engine/Navmesh/NavMeshThreaded.h"

class NavJobHandler
{
public:
	NavJobHandler(NavMeshThreaded* aNavMeshThreaded = nullptr) : myNavMeshThreaded(aNavMeshThreaded) {}
	void SetNavMeshThreaded(NavMeshThreaded& aNavMeshThreaded) { myNavMeshThreaded = &aNavMeshThreaded; }

	/**
	 * \brief 
	 */
	void Queue(Vec3f start, Vec3f end);
	
	/**
	 * \brief Call this while waiting in queue to update job status
	 */
	void Tick();

	/**
	 * \brief If your job is finished and ready to be used
	 */
	bool JobDone() const
	{
		return myJobID == -1 && myHasFinishedJob;
	}
	bool InQueue() const
	{
		return myJobID > 0;
	}
	/**
	 * \brief Returns weather or not the handler is empty, no job and not in queue
	 */
	bool Empty() const
	{
		return myJobID == -1 && !myHasFinishedJob;
	}
	

	void ClearJob()
	{
		myNavMeshThreaded->CancelJob(myJobID);
		myJobID = -1;
		myHasFinishedJob = false; 
	};
	 
	NavMeshThreaded::NavMeshJob& GetJobb() { myJobID = -1; myHasFinishedJob = false; return myJob; }
private:
	NavMeshThreaded* myNavMeshThreaded = nullptr;

	int myJobID = -1;

	bool myHasFinishedJob = false;
	NavMeshThreaded::NavMeshJob myJob;
};

