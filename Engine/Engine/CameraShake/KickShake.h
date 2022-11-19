#pragma once

#include "CameraShakeBase.h"

namespace Engine
{
	struct KickShakeDesc
	{
		Displacement myAxesStrength = Displacement({ 0, 0, 0 }, { 1, 1, 1 });

		float myAttackTime = 0.05f;

		float myReleaseTime = 0.2f;

		void InitFromJson(const nlohmann::json& aJson);
	};

	class KickShake : public CameraShakeBase
	{
	public:
		KickShake(const KickShakeDesc& aDesc, const Displacement& aDirection);

		void Update(const float aDeltaTime) override;

	private:
		void Move(const float aDeltaTime, const float aDuration);

	private:
		const KickShakeDesc myDesc;

		float myT = 0.f;

		Displacement myPreviousWaypoint;
		Displacement myCurrentWaypoint;

		bool myRelease = false;

		// 
		// Displacement myDirection;
	};
}