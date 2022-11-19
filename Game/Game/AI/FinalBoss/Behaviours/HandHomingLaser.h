#pragma once

#include "Behaviour.h"

namespace Engine
{
	class VFXComponent;
}

namespace FB
{
	class RightProjectileHand;
	class HandHomingProjectileLauncher;
	struct HomingLaserDesc;

	class HandHomingLaser : public Behaviour
	{
	public:
		HandHomingLaser(RightProjectileHand& aRightHand);

		void Update() override;

		bool IsFinished() const override;

		void OnEnter() override;
		void OnExit() override;

		void Reflect(Engine::Reflector& aReflector) override;

		bool IsAllowedToMoveExternally() const override;

	private:
		void OnHomingLaserShoot();
		void SpawnBuildupVFX();
		Vec3f GetOutputPosition() const;

		const HomingLaserDesc& Desc() const;
		void StopBuildupVFX();
		void HomingLaserSound(const bool aEnable);

	private:
		RightProjectileHand& myRightHand;
		const HomingLaserDesc& myDesc;

		bool myIsFinished = false;

		Engine::VFXComponent* myBuilupVFX = nullptr;

		HandHomingProjectileLauncher* myLauncher = nullptr;
	};
}