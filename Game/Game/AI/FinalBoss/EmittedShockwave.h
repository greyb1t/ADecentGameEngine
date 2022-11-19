#pragma once

#include "Engine/GameObject/Components/Component.h"

namespace FB
{
	class EmittedShockwave : public Component
	{
	public:
		COMPONENT(EmittedShockwave, "FB EmittedShockwave");

		void Start();

		void Execute(Engine::eEngineOrder aOrder) override;

		void Reflect(Engine::Reflector& aReflector) override;

		void SetDamage(const float aDamage);
		void SetSpeed(const float aSpeed);
		void SetKnockbarStrength(const float aKnockbackStrength);
		void SphereThiccness(const float aSphereThiccness);
		void SetShockwaveHeight(const float aHeight);

	private:
		void CheckHitPlayer();
		void UpdateSize();

	private:
		float mySpeed = 100.f;
		float myKnockbackStrength = 100.f;
		float mySphereThiccness = 100.f;
		float myShockwaveHeight = 100.f;

		float myRadius = 0.f;

		float myDamage = 0.f;

		Engine::GameObjectRef myModel;

		bool myHasDamagedPlayer = false;
	};
}