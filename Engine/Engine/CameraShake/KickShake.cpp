#include "pch.h"
#include "KickShake.h"

Engine::KickShake::KickShake(const KickShakeDesc& aDesc, const Displacement& aDirection)
	: myDesc(aDesc)
{
	myCurrentWaypoint = Displacement::Scale(aDirection, myDesc.myAxesStrength);
}

static float easeInOutSine(float x)
{
	return -(cos(C::PI * x) - 1) / 2;
}

void Engine::KickShake::Update(const float aDeltaTime)
{
	if (myT < 1.f)
	{
		Move(aDeltaTime, myRelease ? myDesc.myReleaseTime : myDesc.myAttackTime);
	}
	else
	{
		myDisplacement = myCurrentWaypoint;
		myPreviousWaypoint = myCurrentWaypoint;

		if (myRelease)
		{
			myIsFinished = true;
			return;
		}
		else
		{
			myRelease = true;
			myT = 0.f;
			myCurrentWaypoint = Displacement();
		}
	}
}

void Engine::KickShake::Move(const float aDeltaTime, const float aDuration)
{
	if (aDuration > 0.f)
	{
		myT += aDeltaTime / aDuration;
	}
	else
	{
		myT = 1.f;
	}

	myDisplacement = Displacement::Lerp(myPreviousWaypoint, myCurrentWaypoint, easeInOutSine(myT));
}

void Engine::KickShakeDesc::InitFromJson(const nlohmann::json& aJson)
{
	myAxesStrength.InitFromJson(aJson["AxesStrength"]);

	myAttackTime = aJson["AttackTime"];
	myReleaseTime = aJson["ReleaseTime"];
}
