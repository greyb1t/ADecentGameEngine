#pragma once

#include "AI/FinalBoss/States/FinalBossState.h"
#include "Common/Math/BezierSpline.h"
#include "Utils/TickTimer.h"

namespace Engine
{
	class Curve;
}

namespace FB
{
	class HandPunch;
	struct HandPunchDesc;

	class HandPunch_MoveTowardsPlayer : public BaseFinalBossState
	{
	public:
		HandPunch_MoveTowardsPlayer(HandPunch& aParent);

		void Update() override;

		bool IsFinished() override;

		void OnEnter() override;
		void OnExit() override;

		void OnCollidedWithObstacle();

	private:
		void SpawnTrailVFX();
		const HandPunchDesc& Desc() const;

		void HitGroundShake();
	private:
		HandPunch& myHandPunch;
		TickTimer myTimer;
		TickTimer myInitialDelay;
		BezierSpline myBezier;
		Engine::Curve* mySpeedCurve = nullptr;
		bool myCollidedWithObstacle = false;

		bool myPlayedSound = false;
		void SpawnCollidedVFX();
	};
}