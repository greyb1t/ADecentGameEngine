#include "pch.h"
#include "BounceShake.h"

Engine::BounceShake::BounceShake(const BounceShakeDesc& aDesc)
	: myDesc(aDesc)
{
	auto rnd = Displacement::InsideUnitSphere();
	myDirection = Displacement::Scale(rnd, aDesc.myAxesMultiplier).Normalized();

	myCurrentWaypoint = myDirection.ScaledBy(myDesc.myPositionStrength, myDesc.myRotationStrength);
}

static float easeInOutSine(float x)
{
	return -(cos(C::PI * x) - 1) / 2;
}

void Engine::BounceShake::Update(const float aDeltaTime)
{
	if (myT < 1.f)
	{
		myT += aDeltaTime * myDesc.myFrequency;

		// ?
		if (myDesc.myFrequency == 0.f)
		{
			myT = 1.f;
		}

		myDisplacement = Displacement::Lerp(
			myPreviousWaypoint,
			myCurrentWaypoint,
			easeInOutSine(myT));
	}
	else
	{
		myT = 0.f;

		myDisplacement = myCurrentWaypoint;
		myPreviousWaypoint = myCurrentWaypoint;

		++myBouncesCount;

		// Finished?
		if (myBouncesCount > myDesc.myBounceCount)
		{
			myIsFinished = true;
			return;
		}

		const auto rnd = Displacement::InsideUnitSphere();

		myDirection = myDirection * -1.f + Displacement::Scale(rnd, myDesc.myAxesMultiplier).Normalized() * myDesc.myRandomness;

		const float percentage = static_cast<float>(myBouncesCount) / static_cast<float>(myDesc.myBounceCount);
		const float decayValue = 1.f - percentage;

		myCurrentWaypoint = decayValue * decayValue * myDirection.ScaledBy(myDesc.myPositionStrength, myDesc.myRotationStrength);
	}
}
