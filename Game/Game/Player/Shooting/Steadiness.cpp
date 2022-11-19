#include "pch.h"
#include "Steadiness.h"

Steadiness::Steadiness(float aSteadinessRecover)
	: mySteadinessRecover(aSteadinessRecover)
{
}

void Steadiness::Update()
{
	mySteadiness = std::max(0.f, mySteadiness - Time::DeltaTime * mySteadinessRecover);
}

void Steadiness::Add(float aValue)
{
	mySteadiness = std::min(1.f, mySteadiness + aValue);
}

void Steadiness::Recover(float aValue)
{
	mySteadiness = std::max(0.f, aValue);
}

float Steadiness::Get() const
{
	return mySteadiness;
}