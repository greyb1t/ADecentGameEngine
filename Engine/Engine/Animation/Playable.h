#pragma once

#include "AnimationTransformResult.h"

namespace Engine
{
	class AnimationState;
	struct AnimationEvent;
	class AnimationMachine;
	class PlayableVisitor;

	class Playable
	{
	public:
		Playable();
		virtual ~Playable();

		virtual void InitFromJson2(const nlohmann::json& aJson, AnimationMachine& aMachine) = 0;
		virtual nlohmann::json ToJson() const = 0;

		virtual void Accept(PlayableVisitor& aVisitor) { }

		virtual AnimationTransformResult CalculateInterpolatedTransforms() = 0;

		virtual AnimationTransform GetRootMotionDelta() const = 0;

		virtual bool HasFinished() const = 0;
		virtual void Reset() = 0;

		virtual float GetElapsedTimeNormalized() const = 0;
		virtual void SetElapsedTimeNormalized(const float aTimeNormalized) = 0;

		virtual float GetDurationSeconds() const = 0;
		virtual bool IsRootMotion() const = 0;
		virtual void SetIsRootMotion(const bool aIsRootMotion) = 0;

		virtual void SetTimeScale(const float aTimeScale) = 0;
		virtual float GetTimeScale() const = 0;

		virtual void Update(AnimationState& aState, const float aDeltaTime) = 0;

		virtual const std::string& GetName() const = 0;

		virtual std::vector<const AnimationEvent*> GetEventsExecutedThisFrame() = 0;

		virtual bool IsLooping() const = 0;
		virtual void SetIsLooping(const bool aIsLooping) = 0;

		virtual bool CanPlay() const = 0;

		virtual void RemoveTriggerUsage(const std::string& aTriggerName) { }
		virtual void RemoveConditionUsage(const std::string& aConditionName) { }
		virtual void RemoveFloatUsage(const std::string& aFloatName) { }
		virtual void RemoveIntUsage(const std::string& aIntName) { }
		virtual void RemoveVec2Usage(const std::string& aVec2Name) { }
	};
}