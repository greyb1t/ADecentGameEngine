#include "pch.h"
#include "TimerManager.h"

void TimerManager::Update()
{
    myCurrentTime += Time::DeltaTime;
    for (int i = myTimers.size() -1; i >= 0; --i)
    {
        if (myTimers[i].endTime < myCurrentTime)
        {
            myTimers[i].myCallback();
            myTimers.pop_back(); 
        }
        else
        {
            break;
        }
    }
}

int TimerManager::SetTimer(float aDuration, const Callback& aCallback)
{
    Timer timer;
    timer.endTime = myCurrentTime + aDuration;
    timer.ID = myNextID;
    timer.myCallback = aCallback;
    bool inserted = false;
    for (auto it = myTimers.begin(); it != myTimers.end(); ++it)
    {
        if (timer.endTime > it->endTime)
        {
            myTimers.insert(it, timer);
            inserted = true;
            break;
        }
    }
    if (!inserted)
    {
        myTimers.push_back(timer);
    }

    myNextID++;
    return myNextID - 1;
}

float TimerManager::CheckTimer(int anID)
{
    for (auto& timer : myTimers)
    {
        if (timer.ID == anID)
        {
            return timer.endTime - myCurrentTime;
        }
    }
    return -1.f;
}

void TimerManager::StopTimer(int anID)
{
    for (int i = 0; i < myTimers.size(); ++i)
    {
        if (myTimers[i].ID == anID)
        {
            myTimers.erase(myTimers.begin() + i);
            --i;
        }
    }
}
