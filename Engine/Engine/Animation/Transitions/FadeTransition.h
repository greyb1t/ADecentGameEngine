#pragma once

#include "Transition.h"

namespace Engine
{
	class AnimationState;
	class AnimationController;

	class FadeTransition : public Transition
	{
	public:
		FadeTransition();

		FadeTransition(
			const std::string& aTargetStateName,
			const float aDurationSeconds,
			const float aExitTimeNormalized,
			bool aCanInterruptCurrentPlayingAnimation)
			: Transition(
				aTargetStateName,
				aExitTimeNormalized,
				aCanInterruptCurrentPlayingAnimation),
			myDurationSeconds(aDurationSeconds)
		{
		}

		void InitFromJson(const nlohmann::json& aJson) override;
		nlohmann::json ToJson() const override;

		float GetDurationSeconds() const;

		void Reset() override;

		TransitionStatus Update(AnimationController& aController, const float aDeltaTime) override;

		TransitionResult CalculateTransforms(
			const AnimationTransformResult& aTransformResult) override;

		void DoTransition(AnimationMachineLayer& aLayer) override;

	private:
		friend class AnimationNodeEditorWindow;
		friend class AnimationState;

		float myFadeTimerSeconds = 0.f;
		float myDurationSeconds = 0.f;
	};
}