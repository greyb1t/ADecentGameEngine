#pragma once

#include "Playable.h"
#include "AnimationEvent.h"
#include "Engine/ResourceManagement\ResourceRef.h"

namespace Engine
{
	class AnimationClip;
	class SkinnedModel;
}

namespace Engine
{
	class AnimationState;

	class NormalAnimation : public Playable
	{
	public:
		NormalAnimation();

		nlohmann::json ToJson() const override;
		void InitFromJson2(const nlohmann::json& aJson, AnimationMachine& aMachine) override;

		bool InitFromJson(const nlohmann::json& aJson);

		AnimationTransformResult CalculateInterpolatedTransforms() override;

		AnimationTransform GetRootMotionDelta() const override;

		void Update(AnimationState& aState, const float aDeltaTime) override;

		void SetTimeScale(const float aTimeScale) override;
		float GetTimeScale() const override;

		void SetIsLooping(const bool aIsLooping) override;
		bool IsLooping() const override;

		bool IsRootMotion() const override;
		void SetIsRootMotion(const bool aIsRootMotion) override;

		bool HasFinished() const override;
		void Reset() override;

		float GetElapsedTimeNormalized() const override;
		float GetCurrentFrame() const;

		float GetDurationSeconds() const override;

		void SetElapsedTimeNormalized(const float aTimeNormalized) override;

		const std::string& GetName() const override;

		std::vector<const AnimationEvent*> GetEventsExecutedThisFrame() override;

		bool CanPlay() const override;

		void Accept(PlayableVisitor& aVisitor) override;

	private:
		friend class AnimationNodeEditorWindow;
		friend class FindAnimationClipsVisitor;
		friend class BlendTree2DEditor;

		AnimationClipRef myClip;

		std::string myName;

		float myAnimTimeInTicks = 0.f;

		float myTimeScale = 1.f;

		bool myIsLooping = false;

		AnimationTransform myRootMotion;
		AnimationTransform myRootMotionPrevious;

		bool myIsRootMotion = false;

		std::vector<AnimationEvent> myEvents;

		std::vector<const AnimationEvent*> myEventsExecutedThisFrame;
		std::vector<AnimationEvent*> myEventsNotExecuted;
	};
}