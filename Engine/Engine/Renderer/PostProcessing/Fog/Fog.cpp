#include "pch.h"
#include "Fog.h"

Engine::Fog::Fog(const std::string& aName)
	: myName(aName)
{

}

void Engine::Fog::SetData(const FogData& aData)
{
	myData = aData;
}

const std::string& Engine::Fog::GetName() const
{
	return myName;
}

const Engine::FogData& Engine::Fog::GetData() const
{
	return myData;
}

Engine::FogData Engine::FogData::Lerp(const FogData& aFirst, const FogData& aSecond, const float aT)
{
	FogData result;

	result.myColor = Math::Lerp(aFirst.myColor, aSecond.myColor, aT);

	result.myStart = Math::Lerp(aFirst.myStart, aSecond.myStart, aT);
	result.myEnd = Math::Lerp(aFirst.myEnd, aSecond.myEnd, aT);

	result.myFogStrength = Math::Lerp(aFirst.myFogStrength, aSecond.myFogStrength, aT);

	return result;
}
