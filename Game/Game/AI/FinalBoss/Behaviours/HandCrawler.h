#pragma once

#include "Behaviour.h"
#include "AI/FinalBoss/StateSequencer.h"
#include "AI/FinalBoss/States/FinalBossState.h"
#include "AI/FinalBoss/FinalBossDamage.h"

// Only purpose of this class is to create a common interface for the handcrawler
// that the states can use to avoid duplicating code for both left and right hands

namespace Engine
{
	class AnimatorComponent;
}

namespace FB
{
	BETTER_ENUM(HandCrawlerStates, int,
		Start,
		Loop,
		End);

	struct HandCrawlerDesc : public Engine::Reflectable
	{
		float myStopSamplingPlayerPosDistance = 1000.f;
		float myMoveSpeed = 1000.f;
		float myOffsetAboveGround = 200.f;

		Vec2f myHuntDurationRange = Vec2f(10.f, 20.f);

		Vec2f myNextZigZagPositionDistanceMinMax = Vec2f(350.f, 800.f);

		Vec2f myNextZigZagAngleOffsetRange = Vec2f(-Math::PI * 0.5f, Math::PI * 0.5f);

		AnimationCurveRef myDecentSpeedCurve;

		int myMaxZigZagAttemptsMin = 3;
		int myMaxZigZagAttemptsMax = 7;

		AnimationCurveRef myJumpSpeedCurve;

		float myRotationSpeed = 10.f;

		FinalBossDamage myDamage;
		float myKnockbackForce = 1000.f;

		void Reflect(Engine::Reflector& aReflector) override
		{
			aReflector.Reflect(myStopSamplingPlayerPosDistance, "Stop Sampling PlayerPos Distance");
			aReflector.Reflect(myMoveSpeed, "Move Speed");
			aReflector.Reflect(myHuntDurationRange, "Hunt Duration Seconds Range");
			aReflector.Reflect(myOffsetAboveGround, "Offset Above Ground");
			aReflector.Reflect(myDecentSpeedCurve, "Decent Speed Curve");
			aReflector.Reflect(myNextZigZagPositionDistanceMinMax, "Next ZigZag Position Distance Min Max");
			aReflector.Reflect(myMaxZigZagAttemptsMin, "Max ZigZag Attempts Min");
			aReflector.Reflect(myMaxZigZagAttemptsMax, "Max ZigZag Attempts Max");
			myMaxZigZagAttemptsMin = std::max(myMaxZigZagAttemptsMin, 0);
			if (myMaxZigZagAttemptsMin > myMaxZigZagAttemptsMax)
			{
				myMaxZigZagAttemptsMin = myMaxZigZagAttemptsMax;
			}
			aReflector.Reflect(myNextZigZagAngleOffsetRange, "ZigZag Angle Offset MinMax");
			aReflector.Reflect(myJumpSpeedCurve, "Jump Towards Player Speed Curve");
			aReflector.Reflect(myDamage, "Damage");
			aReflector.Reflect(myKnockbackForce, "Knockback Force");
			aReflector.Reflect(myRotationSpeed, "Rotation Speed");
		}
	};

	class HandCrawler : public Behaviour
	{
	public:
		HandCrawler(FinalBossHand& aBossHand);
		virtual ~HandCrawler() = default;

		void Update() override;

		bool IsFinished() const override;

		void OnEnter() override;
		void OnExit() override;

		void Reflect(Engine::Reflector& aReflector) override;

		FinalBossHand& GetBossHand();

		bool IsAllowedToMoveExternally() const override { return false; }

		const HandCrawlerDesc& Desc() const;

	private:
		FinalBossHand& myBossHand;
		StateSequencer myStateSequencer;

		std::array<Owned<BaseFinalBossState>, HandCrawlerStates::_size()> myStates;
	};
}