#include "pch.h"
#include "Transition.h"
#include "..\State\AnimationMachine.h"
#include "Engine/Animation/State/AnimationMachineLayer.h"

Engine::Transition::Transition()
{

}

Engine::Transition::~Transition()
{
}

void Engine::Transition::InitFromJson(const nlohmann::json& aJson)
{
	AnimationNodeEditorWindow::Link::InitFromJson(aJson["Link"]);

	if (aJson.contains("TransitionName"))
		myTransitionName = aJson["TransitionName"];
	else
		LOG_ERROR(LogType::Animation) << "Missing \"TransitionName\"";

	if (aJson.contains("TargetStateName"))
		myTargetStateName = aJson["TargetStateName"];
	else
		LOG_ERROR(LogType::Animation) << "Missing \"TargetStateName\" in" << myTransitionName;

	if (aJson.contains("StartAt"))
	{
		myExitTimeNormalized = aJson["StartAt"];
	}
	else
	{
		LOG_ERROR(LogType::Animation) << "Missing \"StartAt\" in" << myTransitionName;
	}

	if (aJson.contains("CanInterruptCurrentPlayingAnimation"))
		myCanInterruptCurrentPlayingAnimation = aJson["CanInterruptCurrentPlayingAnimation"];
	else
		LOG_ERROR(LogType::Animation) << "Missing \"CanInterruptCurrentPlayingAnimation\" in " << myTransitionName;

	if (aJson.contains("TriggerName"))
	{
		myTriggerName = aJson["TriggerName"];
	}

	if (aJson.contains("Conditions") && !aJson["Conditions"].is_null())
	{
		ConditionGroup conditionGroup;

		for (const nlohmann::json& conditionJson : aJson["Conditions"])
		{
			Condition condition;
			condition.InitFromJson(conditionJson);

			conditionGroup.AddCondition(condition);
		}

		myConditionGroups.push_back(conditionGroup);
	}

	if (aJson.contains("ConditionGroups"))
	{
		for (const nlohmann::json& conditionJson : aJson["ConditionGroups"])
		{
			ConditionGroup condGroup;
			condGroup.InitFromJson(conditionJson);

			myConditionGroups.push_back(condGroup);
		}
	}
}

nlohmann::json Engine::Transition::ToJson() const
{
	nlohmann::json j;

	j["Link"] = AnimationNodeEditorWindow::Link::ToJson();

	j["TransitionName"] = myTransitionName;

	j["TargetStateName"] = myTargetStateName;

	j["StartAt"] = myExitTimeNormalized;

	j["CanInterruptCurrentPlayingAnimation"] = myCanInterruptCurrentPlayingAnimation;

	j["TriggerName"] = myTriggerName;

	nlohmann::json condGroupsArrayJson;

	for (const auto& condGroup : myConditionGroups)
	{
		condGroupsArrayJson.push_back(condGroup.ToJson());
	}

	j["ConditionGroups"] = condGroupsArrayJson;

	/*
	nlohmann::json conditionsArrayJson;

	for (const auto& condition : myConditions)
	{
		conditionsArrayJson.push_back(condition.ToJson());
	}

	j["Conditions"] = conditionsArrayJson;
	*/

	return j;
}

Engine::Transition::Transition(
	const std::string& aTargetStateName,
	const float aExitTimeNormalized,
	bool aCanInterruptCurrentPlayingAnimation)
	: myTargetStateName(aTargetStateName),
	myExitTimeNormalized(aExitTimeNormalized),
	myCanInterruptCurrentPlayingAnimation(aCanInterruptCurrentPlayingAnimation)
{

}

void Engine::Transition::Reset()
{
	myHasStartedTransitioning = false;

	//if (myTrigger)
	//{
	//	// Reset the trigger after the transition finished
	//	*myTrigger = false;
	//}
}

Engine::AnimationState* Engine::Transition::GetTargetState()
{
	return myTargetState;
}

const std::string& Engine::Transition::GetTargetStateName() const
{
	return myTargetStateName;
}

float Engine::Transition::GetExitTimeNormalized() const
{
	return myExitTimeNormalized;
}

void Engine::Transition::SetHasStartedTransitioning(const bool aHasStartedTransitioning)
{
	myHasStartedTransitioning = aHasStartedTransitioning;
}

bool Engine::Transition::HasStartedTransitioning() const
{
	return myHasStartedTransitioning;
}

void Engine::Transition::ActivateTrigger()
{
	*myTrigger = true;
}

bool Engine::Transition::IsAnyConditionsTrue() const
{
	if (myConditionGroups.empty())
	{
		return true;
	}

	for (const auto& cond : myConditionGroups)
	{
		if (cond.IsTrue())
		{
			return true;
		}
	}

	return false;
}

bool Engine::Transition::CanDoTransition() const
{
	// NOTE: Currently support ONLY trigger or ONLY conditions
	// NOT BOTH

	if (HasTrigger())
	{
		// Hardcoded fix to support triggers and conditions. If trigger is called, it only transitions if the conditions are true
		return WasTriggered() && IsAnyConditionsTrue();
	}
	else
	{
		// If it does not have a trigger, check only the conditions
		return IsAnyConditionsTrue();
	}
}

bool Engine::Transition::WasTriggered() const
{
	if (HasTrigger())
	{
		return *myTrigger;
	}

	return false;
}

void Engine::Transition::RemoveTriggerUsage(const std::string& aTriggerName)
{
	if (myTriggerName == aTriggerName)
	{
		myTrigger = nullptr;
		myTriggerName.clear();
	}
}

void Engine::Transition::RemoveBoolUsage(const std::string& aConditionName)
{
	for (auto& condGroup : myConditionGroups)
	{
		for (int i = static_cast<int>(condGroup.GetConditions().size()) - 1; i >= 0; --i)
		{
			auto& cond = condGroup.GetConditions()[i];

			if (cond.GetValueType() == ConditionValueType::Bool &&
				cond.GetValueName() == aConditionName)
			{
				condGroup.GetConditions().erase(condGroup.GetConditions().begin() + i);
			}
		}
	}
}

void Engine::Transition::RemoveFloatUsage(const std::string& aFloatName)
{
	for (auto& condGroup : myConditionGroups)
	{
		for (int i = static_cast<int>(condGroup.GetConditions().size()) - 1; i >= 0; --i)
		{
			auto& cond = condGroup.GetConditions()[i];

			if (cond.GetValueType() == ConditionValueType::Float &&
				cond.GetValueName() == aFloatName)
			{
				condGroup.GetConditions().erase(condGroup.GetConditions().begin() + i);
			}
		}
	}
}

void Engine::Transition::RemoveVec2Usage(const std::string& aVec2Name)
{

}

void Engine::Transition::RemoveIntUsage(const std::string& aIntName)
{
	for (auto& condGroup : myConditionGroups)
	{
		for (int i = static_cast<int>(condGroup.GetConditions().size()) - 1; i >= 0; --i)
		{
			auto& cond = condGroup.GetConditions()[i];

			if (cond.GetValueType() == ConditionValueType::Int &&
				cond.GetValueName() == aIntName)
			{
				condGroup.GetConditions().erase(condGroup.GetConditions().begin() + i);
			}
		}
	}
}

bool Engine::Transition::HasTrigger() const
{
	return !myTriggerName.empty();
}

void Engine::Transition::SetIsAnyStateTransition(const bool aIsAnyStateTransition)
{
	myIsAnyStateTransition = aIsAnyStateTransition;
}

bool Engine::Transition::IsAnyStateTransition() const
{
	return myIsAnyStateTransition;
}

bool Engine::Transition::CanInterruptCurrentPlayingAnimation() const
{
	return myCanInterruptCurrentPlayingAnimation;
}

bool Engine::Transition::SetupReferences(AnimationMachineLayer& aLayer)
{
	auto state = aLayer.TryGetState(myTargetStateName);

	if (state)
	{
		myTargetState = &aLayer.GetState(myTargetStateName);

		return true;
	}
	else
	{
		LOG_ERROR(LogType::Animation) << "Cannot find state " << myTargetStateName << " in transition " << myTransitionName;
		return false;
	}
}

const std::string& Engine::Transition::GetTransitionName() const
{
	return myTransitionName;
}

std::vector<Engine::ConditionGroup>& Engine::Transition::GetConditionGroups()
{
	return myConditionGroups;
}

const std::string& Engine::Transition::GetTriggerName() const
{
	return myTriggerName;
}

void Engine::Transition::SetTriggerPtr(bool* aTrigger)
{
	myTrigger = aTrigger;
}

//void Engine::Transition::SetConditionPtr(const std::string& aConditionName, bool* aBoolPointer)
//{
//	myConditions[aConditionName].SetBoolPointer(aBoolPointer);
//}
