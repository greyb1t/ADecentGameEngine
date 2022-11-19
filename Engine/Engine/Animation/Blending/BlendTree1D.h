#pragma once

#include "BlendTree.h"
#include "BlendNode1D.h"

namespace Engine
{
	class BlendTree1D : public BlendTree
	{
	public:
		void AddNode(BlendNode1D aNode);

		AnimationTransformResult CalculateInterpolatedTransforms() override;

		AnimationTransform GetRootMotionDelta() const override;

		bool HasFinished() const override;
		void Reset() override;
		float GetElapsedTimeNormalized() const override;

		void Update(AnimationState& aState, const float aDeltaTime) override;

		float GetDurationSeconds() const override;

		bool IsRootMotion() const override;

		// when adding a node, and later on modifying it, we must
		// call this to recalculate the tree variables
		void RecalculateTree();

		void InitFromJson2(const nlohmann::json& aJson, AnimationMachine& aMachine) override;
		nlohmann::json ToJson() const override;
		void SetElapsedTimeNormalized(const float aTimeNormalized) override;
		void SetIsRootMotion(const bool aIsRootMotion) override;
		void SetTimeScale(const float aTimeScale) override;
		float GetTimeScale() const override;
		const std::string& GetName() const override;
		std::vector<const AnimationEvent*> GetEventsExecutedThisFrame() override;
		bool IsLooping() const override;
		void SetIsLooping(const bool aIsLooping) override;
		bool CanPlay() const override;

		void Accept(PlayableVisitor& aVisitor) override;

	private:
		// The right blendnode will be nullptr if no other node was found to blend with
		// this occurs when the blendvalue is the exact same as one of the
		// blend node thesholds
		std::pair<BlendNode1D*, BlendNode1D*> DetermineNodesToBlend();
		float CalculateBlendWeight(const BlendNode1D& aLeft, const BlendNode1D& aRight) const;

		void RemoveFloatUsage(const std::string& aFloatName) override;

		float GetBlendValue() const;

	private:
		friend class AnimationNodeEditorWindow;
		friend class FindAnimationClipsVisitor;

		std::string myName = "BlendTreeName";

		std::vector<BlendNode1D> myNodes;

		float myMinThreshold = 0.f;
		float myMaxThreshold = 1.f;

		std::string myBlendFloatName;
		float* myBlendValue = nullptr;

		float myNormalizedTime = 0.f;

		// The duration is the time of the shortest animation
		float myDurationSeconds = 0.f;

		bool myIsLooping = false;

		AnimationTransform myRootMotion;
		AnimationTransform myRootMotionPrevious;
	};
}