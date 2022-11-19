#pragma once

#include "State/AnimationMachine.h"
#include "Engine/ResourceManagement/ResourceRef.h"
#include "AnimationTransformResult.h"
#include "AnimationStateBehaviour.h"
#include "Engine/Renderer/Shaders/ShaderConstants.h"

namespace Engine
{
	class ModelInstance;
	class AnimationClip;
	struct OurNode;
	class AnimationSingle;
	struct AnimationEvent;
	class AnimationState;
	class Transition;
	class AnimatorComponent;
	class AnimationStateBehaviour;

	class AnimationController
	{
	public:
		AnimationController();
		~AnimationController();

		void Trigger(const std::string& aTriggerName);
		bool HasTrigger(const std::string& aTriggerName) const;
		bool HasBool(const std::string& aBoolName) const;

		void SetBool(const std::string& aBoolName, const bool aValue);
		bool GetBool(const std::string& aBoolName) const;
		void SetFloat(const std::string& aFloatName, const float aValue);
		float GetFloat(const std::string& aFloatName) const;
		void SetVec2(const std::string& aVecName, const Vec2f& aValue);
		const Vec2f& GetVec2(const std::string& aVecName) const;

		void SetInt(const std::string& anIntName, const int aValue);
		int GetInt(const std::string& anIntName);

		const AnimationMachine& GetAnimationMachine() const;

		void AddEventCallback(const std::string& aEventName, std::function<void()> aCallback);
		void AddStateBehaviour(const std::string& aLayerName, const std::string& aStateName, AnimationStateBehaviour* aStateBehaviour);

		// Added for convenience
		void AddStateOnExitCallback(
			const std::string& aLayerName,
			const std::string& aStateName,
			std::function<void()> aOnExit,
			std::function<void()> aOnEnter = nullptr);

		// Returns -1 if not found
		int GetBoneIndexFromName(const std::string& aBoneName) const;
		Mat4f GetBoneTransformWorld(const int aBoneIndex);
		Mat4f GetBoneTransformWorld(const std::string& aBoneName);

		void SetLayerWeight(const std::string& aLayerName, const float aWeight);
		const float GetLayerWeight(const std::string& aLayerName) const;

		AnimationState* FindState(const std::string& aLayerName, const std::string& aStateName) const;

		float GetTimeScale() const;
		void SetTimeScale(const float aTimeScale);
		void SetLayerTimeScale(const std::string& aLayerName, float aTimeScale);

	private:
		friend class FadeTransition;
		friend class InstantTransition;
		friend class NormalAnimation;
		friend class AnimatorComponent;
		friend class AnimationTab;
		friend class AnimationState;
		friend class AnimationMachineLayer;

		// ALL ANIMATINOS MUST HAVE SAME SKELETON
		void CalculateFinalTransforms(
			const std::vector<OurNode>& aBoneNodes,
			const std::vector<Mat4f>& aLocalTransforms,
			std::array<Mat4f, ShaderConstants::MaxBoneCount>& aFinalTransformOffsetsOut,
			std::array<Mat4f, ShaderConstants::MaxBoneCount>& aFinalTransformWorldSpaceOut) const;

		void AddExecutedEvent(const AnimationEvent* aEvent);
		void AddCallbackToRunThisFrame(const std::function<void()>& aCallback);

		bool Init(
			AnimationMachine& aMachine,
			const std::string& aName);

		void Update(const float aDeltaTime);

		void CallCallbacks();

		void SetModelInstance(ModelInstance& aModelInstance);

		AnimationTransform GetRootMotionDelta() const;

		// DO NOT USE IN GAME, ONLY FOR EDITOR
		AnimationMachine& GetMachine();

		void Reflect(Reflector& aReflector);

	private:
		AnimationMachine* myMachine = nullptr;
		ModelInstance* myModelInstance = nullptr;
		std::vector<const AnimationEvent*> myExecutedEventsThisFrame;
		std::unordered_map<std::string, std::function<void()>> myEventCallbacks;

		std::vector<std::function<void()>> myCallbacksToRunThisFrame;

		std::string myName;

		std::array<Mat4f, ShaderConstants::MaxBoneCount> myFinalBoneTransforms;
		std::array<Mat4f, ShaderConstants::MaxBoneCount> myFinalBoneTransformsWorldSpace;

		// Added for convenience
		std::unordered_map<std::string, Owned<AnimationStateBehaviour>> myStateBehavioursForExitOnly;

		float myTimeScale = 1.f;

		// Just to cache the actual allocation
		mutable std::vector<Mat4f> toRootTransforms;
	};
}