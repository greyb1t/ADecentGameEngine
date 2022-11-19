#pragma once

#include "..\AnimationTransformResult.h"
#include "Condition.h"
#include "Engine/Editor/EditorWindows/AnimationNodeEditorWindow.h"
#include "ConditionGroup.h"

namespace Engine
{
	class AnimationMachineLayer;
	class AnimationController;
	class AnimationState;
	class AnimationMachine;

	enum class TransitionStatus
	{
		Ongoing,
		Finished
	};

	struct TransitionResult
	{
		enum
		{
			Ongoing,
			Finished
		} myTransitionState = {};

		AnimationTransformResult myTransformResult;
	};

	class Transition : public AnimationNodeEditorWindow::Link
	{
	public:
		Transition();

		Transition(
			const std::string& aTargetStateName,
			const float aExitTimeNormalized,
			bool aCanInterruptCurrentPlayingAnimation);

		virtual ~Transition();

		virtual void InitFromJson(const nlohmann::json& aJson);
		virtual nlohmann::json ToJson() const;

		virtual TransitionStatus Update(
			AnimationController& aController,
			const float aDeltaTime) = 0;

		virtual TransitionResult CalculateTransforms(
			const AnimationTransformResult& aTransformResult) = 0;

		virtual void DoTransition(AnimationMachineLayer& aLayer) = 0;

		virtual void Reset();

		AnimationState* GetTargetState();
		const std::string& GetTargetStateName() const;

		float GetExitTimeNormalized() const;

		void SetHasStartedTransitioning(const bool aHasStartedTransitioning);
		bool HasStartedTransitioning() const;

		void ActivateTrigger();

		bool CanDoTransition() const;

		bool CanInterruptCurrentPlayingAnimation() const;

		bool SetupReferences(AnimationMachineLayer& aLayer);

		const std::string& GetTransitionName() const;

		std::vector<ConditionGroup>& GetConditionGroups();
		const std::string& GetTriggerName() const;

		void SetTriggerPtr(bool* aTrigger);
		// void SetConditionPtr(const std::string& aConditionName, bool* aBoolPointer);

		bool HasTrigger() const;

		void SetIsAnyStateTransition(const bool aIsAnyStateTransition);
		bool IsAnyStateTransition() const;

		bool IsAnyConditionsTrue() const;
		bool WasTriggered() const;

		void RemoveTriggerUsage(const std::string& aTriggerName);
		void RemoveBoolUsage(const std::string& aConditionName);
		void RemoveFloatUsage(const std::string& aFloatName);
		void RemoveVec2Usage(const std::string& aVec2Name);
		void RemoveIntUsage(const std::string& aIntName);

	protected:
		friend class AnimationNodeEditorWindow;

		std::string myTransitionName;

		AnimationState* myTargetState = nullptr;
		std::string myTargetStateName;

		// Exit time is an offset to determine a animation "finished" earlier
		// to start a transition earlier before it was finished
		// bool myHasExitTime = false;
		float myExitTimeNormalized = 1.f;

		bool myHasStartedTransitioning = false;

		// All these must be true for the transition to occur
		// std::unordered_map<std::string, Condition> myConditions;
		//std::vector<Condition> myConditions;
		std::vector<ConditionGroup> myConditionGroups;

		bool* myTrigger = nullptr;
		std::string myTriggerName;

		bool myCanInterruptCurrentPlayingAnimation = false;

		bool myIsAnyStateTransition = false;
	};
}
