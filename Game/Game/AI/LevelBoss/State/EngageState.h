#pragma once

#include "BaseState.h"

#include "Engine/GameObject/GameObject.h"
#include "AI/PollingStation/PollingStationComponent.h"

namespace LevelBossStates
{
	class EngageState : public LBS::BaseState
	{
	public:
		EngageState() = delete;
		EngageState(LevelBoss& aBoss);

		void Update() override;

		virtual void OnEnableAttack() {};
		virtual void OnDisableAttack() {};

	protected:
		void TurnTowards(const Vec3f& aTargetPosition, float aSpeed = 7.5f);

		PollingStationComponent* GetPollingStation();

		Vec3f GetPredictedPosition();

	protected:
		virtual void OnUpdate(const Vec3f& aPlayerPosition) {};
		virtual void OnIdle(const Vec3f& aPlayerPosition) {};
		virtual void OnAttack(const Vec3f& aPlayerPosition) {};

		enum class State
		{
			Idle,
			AttackStart,
			AttackLoop
		} myState = State::Idle;

	private:

	};
}

