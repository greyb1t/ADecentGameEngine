#pragma once

#include "..\AnimationTransformResult.h"
#include "Engine/Editor/EditorWindows/AnimationNodeEditorWindow.h"

namespace Engine
{
	class AnimationMachineLayer;
	class Playable;
	class Transition;
	class AnimationMachine;
	class AnimationController;
	class AnimationStateBehaviour;
	struct AnimationEvent;
	class FadeTransition;

	class AnimationState : public AnimationNodeEditorWindow::Node
	{
	public:
		AnimationState(
			const std::string& aStateName,
			Owned<Playable> aHasPose);

		virtual ~AnimationState();

		float GetDurationSeconds() const;

		float GetTimeScale() const;
		void SetTimeScale(const float aTimeScale);

	private:
		nlohmann::json ToJson() const;
		void InitFromJson2(const nlohmann::json& aJson, AnimationMachine& aMachine);

		AnimationTransformResult CalculateInterpolatedTransforms();

		void AddTransition(Owned<Transition> aTransition);

		// If it had an Exit Time from a transition, that is taken into consideration
		bool HasFinished() const;
		float GetElapsedTimeNormalized() const;

		void ResetPlayable();
		void ResetState();

		void Update(const float aDeltaTime);

		std::vector<Owned<Transition>>& GetTransitions();

		const std::string& GetName() const;

		void SetupReferences(AnimationMachineLayer& aLayer);

		bool HasPlayable() const;
		Playable& GetPlayable();

		void SetStateBehaviour(AnimationStateBehaviour* aStateBehaviour);

		void OnEnter();
		void OnExit();

		// Is considered active if it has called OnEnter, but not OnExit
		bool IsActive() const;

		void AddExecutedEvent(const AnimationEvent* aEvent);

		void RemoveTriggerUsage(const std::string& aTriggerName);
		void RemoveBoolUsage(const std::string& aConditionName);
		void RemoveFloatUsage(const std::string& aFloatName);
		void RemoveVec2Usage(const std::string& aVec2Name);
		void RemoveIntUsage(const std::string& aIntName);

		void SetController(AnimationController& aController);

		bool StayWhenFinished() const;

	private:
		friend struct TransitionDesc;
		friend struct FadeTransitionDesc;
		friend struct InstantTransitionDesc;
		friend class AnimationNodeEditorWindow;
		friend class AnimationMachine;
		friend class AnimationMachineLayer;
		friend class FadeTransition;
		friend class AnimationController;

	private:
		AnimationController* myController = nullptr;

		std::string myName;

		Owned<Playable> myPlayable;

		std::vector<Owned<Transition>> myTransitions;

		std::vector<const AnimationEvent*> myExecutedEventsThisFrame;

		AnimationStateBehaviour* myStateBehaviour = nullptr;

		bool myHasEntered = false;
		bool myHasExited = false;

		// When the state has no transition to anything else
		// it by default goes remove the active state and nothing is there
		// This ensures we stay in this state
		bool myStayWhenFinished = false;

		float myTimeScale = 1.f;
	};
}
