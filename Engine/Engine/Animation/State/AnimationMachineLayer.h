#pragma once
#include "Engine/Animation/AnimationTransformResult.h"
#include "Engine/Animation/Blending/BlendMask.h"

namespace Engine
{
	class AnimationMachine;
	class AnimationController;
	class Transition;
	class AnimationState;

	class AnimationMachineLayer
	{
	public:
		enum class Type
		{
			Override = 0,
			Additive = 1,

			Count,
		};

		static std::string TypeToString(const Type aType);

	public:
		AnimationMachineLayer(const std::string& aName);
		~AnimationMachineLayer();

		bool InitFromJson(AnimationMachine& aMachine, const nlohmann::json& aJson);
		nlohmann::json ToJson() const;

		void ResetToInitialState();
		void SetController(AnimationController& aController);

		AnimationTransformResult Update(const float aDeltaTime, AnimationController& aController);
		void UpdateTransitions(AnimationController& aController);

		void SetActiveTransition(Transition* aTransition);

		bool ShouldDoTransition(Transition& aTransition, AnimationController& aController) const;

		void SetStateByName(const std::string& aStateName);

		void SetActiveState(AnimationState* aAnimationState);

		void RemoveActiveState();

		void AddState(Owned<AnimationState> aState);

		AnimationState& GetState(const std::string& aStateName);

		AnimationState* TryGetState(const std::string& aStateName);

		AnimationState* GetActiveState();

		AnimationState& GetAnyState();

		void Setup(const std::string& aInitialStateName);

		bool HasInitialState() const;

		AnimationState& GetInitialState();

		const std::string& GetName() const;

		Type GetType() const;
		const BlendMask& GetBlendMask() const;

		float GetWeight() const;
		void SetWeight(float aWeight);

		void RemoveTriggerUsage(const std::string& aTriggerName);
		void RemoveBoolUsage(const std::string& aConditionName);
		void RemoveFloatUsage(const std::string& aFloatName);
		void RemoveVec2Usage(const std::string& aVec2Name);
		void RemoveIntUsage(const std::string& aIntName);

		void SetTimeScale(const float aTimeScale);

	private:
		friend class AnimationNodeEditorWindow;
		friend class AnimationMachine;

		AnimationState* myInitialState = nullptr;

		AnimationState* myActiveState = nullptr;
		Transition* myActiveTransition = nullptr;

		std::unordered_map<std::string, Owned<AnimationState>> myStates;
		AnimationState* myAnyState = nullptr;

		std::string myName;

		Type myType = Type::Override;

		BlendMask myBlendMask;

		float myWeight = 1.f;

		float myTimeScale = 1.f;
	};
}
