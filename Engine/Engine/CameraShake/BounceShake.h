#pragma once

#include "CameraShakeBase.h"

namespace Engine
{
	struct BounceShakeDesc
	{
		// The strength of shake for the position
		float myPositionStrength = 0.f;

		float myRotationStrength = 0.f;

		Displacement myAxesMultiplier = Displacement({ 1, 1, 1 }, { 0, 0, 1 } /* forward? */);

		float myFrequency = 0.f;

		// Bounces before stopping
		int myBounceCount = 0;

		float myRandomness = 0.f;
	};

	class BounceShake : public CameraShakeBase
	{
	public:
		BounceShake(const BounceShakeDesc& aDesc);

		void Update(const float aDeltaTime) override;

	private:
		const BounceShakeDesc myDesc;

		float myT = 0.f;
		int myBouncesCount = 0;

		Displacement myPreviousWaypoint;
		Displacement myCurrentWaypoint;

		Displacement myDirection;
	};
}