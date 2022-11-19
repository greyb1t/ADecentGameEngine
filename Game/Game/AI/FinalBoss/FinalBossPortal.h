#pragma once

#include "Engine/GameObject/Components/Component.h"
#include "Utils/TickTimer.h"

namespace FB
{
	class FinalBoss;

	class FinalBossPortal : public Component
	{
	private:
		enum class State
		{
			Transitioning,
			Sustaining,
			None
		};

	public:
		COMPONENT(FinalBossPortal, "FB Portal");

		void Start() override;

		void Execute(Engine::eEngineOrder aOrder) override;

		void Reflect(Engine::Reflector& aReflector) override;

		void Pulse(
			const float aIntensity,
			const float aTransitionDurationSec,
			const float aSustainDurationSec);

		void SetPortalColor(const Vec3f& aColor);

	private:
		void SetPortalIntensity(const float aIntensity);

	private:
		Engine::GameObjectRef myPlaneRef;
		Engine::GameObjectRef myFinalBossRef;
		AnimationCurveRef myIntensityCurve;

		FinalBoss* myFinalBoss = nullptr;
		Engine::ModelComponent* myPortalPlaneModel = nullptr;

		Vec2f myPulseTransitionDurationMinMax = Vec2f(0.5f, 2.f);
		Vec2f mySustainIntensityDurationMinMax = Vec2f(0.5f, 2.f);

		Vec2f myIntensityMinMax = Vec2f(0.5f, 10.f);

		float myStartIntensity = 1.f;
		float myTargetIntensity = 1.f;

		TickTimer mySustainTimer;
		TickTimer myTransitionTimer;

		State myState = State::None;
	};
}