#include "pch.h"
#include "Buff.h"

Buff::Buff(const std::function<void(GameObject*, float)>& anEndFunction, GameObject& aGameObject, BuffID aBuffID, float aDuration) :
	myGameObject(&aGameObject),
	myFunction(anEndFunction), 
	myBuffID(aBuffID),
	myRemainingTime(aDuration), 
	myCallsOnTicks(false)
{ }

Buff::Buff(const std::function<void(GameObject*, float)>& aTickFunction, GameObject& aGameObject, BuffID aBuffID, float aDuration, float aTickTime) :
	myGameObject(&aGameObject),
	myFunction(aTickFunction), 
	myBuffID(aBuffID),
	myRemainingTime(aDuration), 
	myTickTime(aTickTime), 
	myTickTimer(aTickTime),
	myCallsOnTicks(true)
{ }

bool Buff::Update(float aDeltaTime)
{
	myRemainingTime -= aDeltaTime;
	if (myCallsOnTicks)
	{
		myTickTimer -= aDeltaTime;
		if (myTickTimer < 0.f)
		{
			myTickTimer += myTickTime;
			Callback();
		}
	}
	if (myRemainingTime < 0.f)
	{
		if (!myCallsOnTicks)
		{
			Callback();
		}
		return true;
	}
	return false;
}

void Buff::AddStack()
{
	myStacks++;
}

void Buff::Refresh(const float aDuration)
{
	myRemainingTime = aDuration;
}

void Buff::Callback()
{
	myFunction(myGameObject, static_cast<float>(myStacks));
}
