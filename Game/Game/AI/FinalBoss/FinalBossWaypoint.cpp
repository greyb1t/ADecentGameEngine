#include "pch.h"
#include "FinalBossWaypoint.h"

FB::FinalBossWaypoint::FinalBossWaypoint(const Vec3f& aPosition)
	: myPosition(aPosition)
{
}

void FB::FinalBossWaypoint::Take()
{
	myIsTaken = true;
}

void FB::FinalBossWaypoint::Release()
{
	myIsTaken = false;
}

bool FB::FinalBossWaypoint::IsTaken() const
{
	return myIsTaken;
}

const Vec3f& FB::FinalBossWaypoint::GetPosition() const
{
	return myPosition;
}
