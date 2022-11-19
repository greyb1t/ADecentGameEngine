#include "pch.h"
#include "NavJobHandler.h"

void NavJobHandler::Queue(Vec3f start, Vec3f end)
{
	if (myJobID != -1)
	{
		ClearJob();
	}

	myJobID = myNavMeshThreaded->StartJob(start, end);
}

void NavJobHandler::Tick()
{
    if (InQueue())
    {
	   /* if (!myNavMeshThreaded->DoJobExist(myJobID))
	    {
            myNavMeshThreaded->CancelJob(myJobID);
            myHasFinishedJob = true;
            myJobID = -1;
	    }
        else if (myNavMeshThreaded->IsJobDone(myJobID))
        {
            myJob = myNavMeshThreaded->BlowJob(myJobID);
            myJobID = -1;
            myHasFinishedJob = true;
        }*/
        if (myNavMeshThreaded->IsJobDone(myJobID))
        {
            myJob = myNavMeshThreaded->BlowJob(myJobID);
            myJobID = -1;
            myHasFinishedJob = true;
        }
    }
}