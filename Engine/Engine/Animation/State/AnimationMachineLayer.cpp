#include "pch.h"
#include "AnimationMachineLayer.h"

#include "AnimationState.h"
#include "Engine/Animation/AnimationController.h"
#include "Engine/Animation/Playable.h"
#include "Engine/Animation/Transitions/Transition.h"
#include "Engine/Renderer/Animation/AnimationClip.h"
#include "Engine/Renderer/Model/ModelInstance.h"
#include "Engine/ResourceManagement/Resources/AnimationClipResource.h"

bool Engine::AnimationMachineLayer::InitFromJson(AnimationMachine& aMachine, const nlohmann::json& aJson)
{
	myStates = std::unordered_map<std::string, Owned<AnimationState>>();
	myActiveState = nullptr;
	myAnyState = nullptr;

	if (aJson.contains("Name"))
	{
		myName = aJson["Name"];
	}

	if (aJson.contains("Weight"))
	{
		myWeight = aJson["Weight"];
	}

	if (aJson.contains("Type"))
	{
		myType = aJson["Type"];
	}

	if (aJson.contains("States"))
	{
		for (const auto& stateJson : aJson["States"])
		{
			const std::string stateName = stateJson["StateName"];

			auto state = MakeOwned<AnimationState>(stateName, /*hasPose*/nullptr);
			state->InitFromJson2(stateJson, aMachine);

			if (stateName == "Any State")
			{
				myAnyState = state.get();

				for (const auto& transition : state->myTransitions)
				{
					// Give warning if it has conditions, because those are not to be used with AnyState
					if (!transition->GetConditionGroups().empty())
					{
						LOG_ERROR(LogType::Animation) << "The transitions \"" << transition->GetTransitionName() <<
							"\" from AnyState has conditions, only Trigger are to be used with AnyState.";
					}

					transition->SetIsAnyStateTransition(true);

					// myAnyStateTransitions.push_back(std::move(transition));
				}
			}

			AddState(std::move(state));
		}
	}

	if (aJson.contains("BlendMask"))
	{
		myBlendMask.InitFromJson(aJson["BlendMask"]);
	}
	else
	{
		myBlendMask.Init(aMachine.GetFirstAnimationClip());
	}

	if (aJson.contains("EntryStateName"))
	{
		Setup(aJson["EntryStateName"]);
	}
	else
	{
		Setup("");
	}

	return true;
}

std::string Engine::AnimationMachineLayer::TypeToString(const Type aType)
{
	switch (aType)
	{
	case Type::Override: return "Override";
	case Type::Additive: return "Additive";

	default:
		break;
	};

	return "Unhandled case in TypeToString()";
}

void Engine::AnimationMachineLayer::RemoveTriggerUsage(const std::string& aTriggerName)
{
	for (auto& [_, state] : myStates)
	{
		state->RemoveTriggerUsage(aTriggerName);
	}
}

void Engine::AnimationMachineLayer::RemoveBoolUsage(const std::string& aConditionName)
{
	for (auto& [_, state] : myStates)
	{
		state->RemoveBoolUsage(aConditionName);
	}
}

void Engine::AnimationMachineLayer::RemoveFloatUsage(const std::string& aFloatName)
{
	for (auto& [_, state] : myStates)
	{
		state->RemoveFloatUsage(aFloatName);
	}
}

void Engine::AnimationMachineLayer::RemoveVec2Usage(const std::string& aVec2Name)
{
	for (auto& [_, state] : myStates)
	{
		state->RemoveVec2Usage(aVec2Name);
	}
}

void Engine::AnimationMachineLayer::RemoveIntUsage(const std::string& aIntName)
{
	for (auto& [_, state] : myStates)
	{
		state->RemoveIntUsage(aIntName);
	}
}

void Engine::AnimationMachineLayer::SetTimeScale(const float aTimeScale)
{
	myTimeScale = aTimeScale;
}

Engine::AnimationMachineLayer::AnimationMachineLayer(const std::string& aName)
	: myName(aName)
{
}

Engine::AnimationMachineLayer::~AnimationMachineLayer()
{
}

nlohmann::json Engine::AnimationMachineLayer::ToJson() const
{
	nlohmann::json j;

	j["Name"] = myName;
	j["Type"] = myType;
	j["Weight"] = myWeight;

	nlohmann::json statesJson = nlohmann::json::array();

	for (const auto& [name, state] : myStates)
	{
		statesJson.push_back(state->ToJson());
	}

	j["States"] = statesJson;

	j["BlendMask"] = myBlendMask.ToJson();

	if (myInitialState)
	{
		j["EntryStateName"] = myInitialState->GetName();
	}

	return j;
}

void Engine::AnimationMachineLayer::ResetToInitialState()
{
	myActiveTransition = nullptr;
	myActiveState = nullptr;

	if (!HasInitialState())
	{
		return;
	}

	auto& initialState = GetInitialState();

	initialState.ResetPlayable();
	initialState.ResetState();
	initialState.OnEnter();

	SetActiveState(&initialState);
}

void Engine::AnimationMachineLayer::SetController(AnimationController& aController)
{
	for (auto& [name, state] : myStates)
	{
		state->SetController(aController);
	}
}

Engine::AnimationTransformResult Engine::AnimationMachineLayer::Update(const float aDeltaTime, AnimationController& aController)
{
	ZoneNamedN(zone1, "AnimationLayer::Update", true);
	//ZoneTextV(zone1, myName.c_str(), myName.size());
	if (myActiveState)
	{
		std::string s = std::string("Layer: ") + myName + std::string(" State: ") + myActiveState->GetName();
		ZoneTextV(zone1, s.c_str(), s.size());
	}

	AnimationTransformResult localTrans;

	// This helps performance, however, it makes it so when we jump with player
	// the first time, I wont trigger?
	// if (myWeight <= 0.f)
	// {
	// 	return {};
	// }

	if (myActiveState && myActiveState->HasPlayable())
	{
		myActiveState->Update(aDeltaTime * myTimeScale);

		if (aController.myModelInstance->PassedRenderCheck())
		{
			ZoneNamedN(zone1, "AnimationLayer::Calculate", true);
			localTrans = myActiveState->CalculateInterpolatedTransforms();
		}
	}

	// remove multiple transitions, only have one,
// then add any state last to override everything, it is highest priority.

	//if (myActiveTransition)
	//{
	//	// If came here, forgot the playable in the state!
	//	assert(myActiveTransition->GetTargetState()->HasPlayable());
	//}

	if (myActiveTransition != nullptr)
	{
		// Update the transition & the animation associated with the transition
		TransitionStatus transitionStatus = myActiveTransition->Update(aController, aDeltaTime * myTimeScale);

		//const auto transitionResult = myActiveTransition->CalculateTransforms(localTrans);
		// transitionResult.myTransformResult = res2.myTransformResult;

		TransitionResult transitionResult;

		if (aController.myModelInstance->PassedRenderCheck())
		{
			// NOTE(filip): temp way to fix the problem when making a transition from a state with no playable
			if (!localTrans.myEntries.empty())
			{
				transitionResult = myActiveTransition->CalculateTransforms(localTrans);
			}
		}

		switch (transitionStatus)
		{
		case TransitionStatus::Finished:
		{
			ZoneNamedN(zoneFinished, "AnimationLayer::Update::Finished", true);

			if (myActiveTransition->GetTargetState()->GetName() == "Dash Land")
			{
				int test = 0;
			}

			SetActiveState(myActiveTransition->GetTargetState());

			// LOG_INFO(LogType::Animation) << "[" << aController.myName << "] Fade Finished: " <<
			//	myActiveTransition->GetTargetState()->GetName();

			// Reset all the transitions for active state when a transition finished
			// Why?
			// Bcuz when 2 or more transitions occur at same time, we
			// set the other's to HasStarted, therefore we need to reset them here
			// otherwise they would not start at all

			for (auto& transition : myActiveState->GetTransitions())
			{
				transition->Reset();
			}

			myActiveTransition->Reset();

			// Remove it from being active
			myActiveTransition = nullptr;
		} break;

		case TransitionStatus::Ongoing:
		{
		} break;

		default:
			assert(false && "unhandled");
			break;
		}

		localTrans = transitionResult.myTransformResult;
	}

	ZoneNamedN(zoneFinishedsadfsaef, "AnimationLayer::Update::AfterTransition", true);
	if (myActiveState && myActiveState->HasPlayable())
	{
		if (aController.myModelInstance->PassedRenderCheck())
		{
			// NOTE: Currently only supporting root motion on the active state animation
			// no blending and such other shite
			if (myActiveState->GetPlayable().IsRootMotion())
			{
				if (localTrans.myEntries.size() > 1)
				{
					localTrans.myEntries[1].myTranslation = {};
					localTrans.myEntries[1].myRotation = {};
					localTrans.myEntries[1].myScale = { 1.f, 1.f, 1.f };
				}
			}
		}
	}

	ZoneNamedN(zoneUpdate, "AnimationLayer::Update::UpdateTransitions", true);
	UpdateTransitions(aController);

	return localTrans;
}

void Engine::AnimationMachineLayer::UpdateTransitions(AnimationController& aController)
{
	ZoneNamedN(scope1, "AnimationController::UpdateTransitions", true);

	// First we check if any transitions from "Any State" can be done
	// they are prioritized, then we check the normal transitions from
	// the active state
	{
		ZoneNamedN(scope2, "AnimationController::1", true);
		for (const auto& transition : GetAnyState().GetTransitions())
		{
			if (ShouldDoTransition(*transition, aController))
			{
				Transition* previousActiveTransition = myActiveTransition;

				transition->DoTransition(*this);

				if (previousActiveTransition)
				{
					previousActiveTransition->Reset();
				}

				LOG_INFO(LogType::Animation) << "PREVIOUS IS FROM ANYSTATE";

				// Do not do anything else
				return;
			}
		}
	}

	// If no active state, dont do the stuff below
	if (myActiveState == nullptr)
	{
		return;
	}

	const auto& activeStateTransitions = myActiveState->GetTransitions();

	// Used for layers where there is no default state
	// e.g. simply an additive layer that has a "Aim" additive state
	// When we reach a state that has no transitions, and it finished
	// we must remove the active state to avoid it from updating
	if (myActiveState && myActiveState->HasPlayable() && myActiveState->GetPlayable().CanPlay())
	{
		if (activeStateTransitions.empty() && myActiveState->HasFinished())
		{
			if (!myActiveState->StayWhenFinished())
			{
				LOG_INFO(LogType::Animation) << "Reached state with no transition, setting active state to null";
				// SetActiveState();

				RemoveActiveState();
			}
		}
	}

	// If any of the transitions where triggered using a Trigger
	// We should take do them, but ONLY if they were triggered
	// This was the solution to the issue when a transition could not be 
	// interrupted when Triggering a transition during another transition 
	// (the goblin intro issue)
	{
		ZoneNamedN(scope3, "AnimationController::2", true);

		for (auto& transition : activeStateTransitions)
		{
			if (transition->WasTriggered() && ShouldDoTransition(*transition, aController))
			{
				Transition* previousActiveTransition = myActiveTransition;

				transition->DoTransition(*this);

				if (previousActiveTransition)
				{
					previousActiveTransition->Reset();
				}

				LOG_INFO(LogType::Animation) << "PREVIOUS IS FROM TRIGGER";

				// Do not do anything else
				return;
			}
		}
	}

	// If a transition was already taken, and it is now active
	// we should not take another transition until that transition
	// is done.
	// Invecklat -Jesper
	// Do not remove this, its important, but only used for condition transitions
	// if something is triggered however, we handle them as anystate and they take priority
	// that is done in the above code
	if (myActiveTransition != nullptr)
	{
		return;
	}

	{
		ZoneNamedN(scope4, "AnimationController::3", true);

		std::vector<Transition*> myTransitionsToDo;

		for (auto& transition : activeStateTransitions)
		{
			if (ShouldDoTransition(*transition, aController))
			{
				myTransitionsToDo.push_back(transition.get());
			}
		}

		if (myTransitionsToDo.size() > 1)
		{
			ZoneNamedN(scope5, "AnimationController::4", true);

			// When multiple transitions are being done at the same time, it can be a valid thing
			// the valid result is to simply to the last transition ONLY
			// We also give warning because it might not be a thing the user want to do

			myTransitionsToDo.front()->DoTransition(*this);

			for (int i = 1; i < myTransitionsToDo.size(); ++i)
			{
				myTransitionsToDo[i]->SetHasStartedTransitioning(true);
			}


			std::string transitionsDoneListString;

			for (const auto& transition : myTransitionsToDo)
			{
				transitionsDoneListString += transition->GetTransitionName() + ", ";
			}

			LOG_WARNING(LogType::Animation) << "[" << aController.myName <<
				"] WARNING Multiple transitions done: " << transitionsDoneListString.c_str();
		}
		else
		{
			ZoneNamedN(scope6, "AnimationController::5", true);

			for (auto& transition : activeStateTransitions)
			{
				if (ShouldDoTransition(*transition, aController))
				{
					// if (myActiveTransition)
					// {
					// 	myActiveTransition->Reset();
					// }

					// myTransitionsToDo.push_back(transition.get());

					transition->DoTransition(*this);
				}
			}
		}
	}

	// TODO: Give warning if its possible to do multiple transitions!
}

void Engine::AnimationMachineLayer::SetActiveTransition(Transition* aTransition)
{
	ZoneNamedN(scope1, "AnimationMachineLayer::SetActiveTransition", true);

	aTransition->Reset();
	aTransition->SetHasStartedTransitioning(true);

	AnimationState* targetState = aTransition->GetTargetState();
	targetState->ResetPlayable();

	// When we start transitioning to another state
	// we consider the target state as the "active state"
	// meaning the previous state, is considered dead and is 
	// only there for the visual blending.
	// No events/callbacks are allowed to run anymore.
	if (myActiveState)
	{
		myActiveState->OnExit();
	}

	// If we interrupt the current active transition that wasn't done
	// we have to make sure to call Exit on because we entered it
	if (myActiveTransition)
	{
		myActiveTransition->GetTargetState()->OnExit();
	}

	// Must reset AFTER OnExit() was called above
	// due to edge case that targetState could be myActiveState
	// Then it would reset itself right before OnExit() which would cause issues
	targetState->ResetState();

	targetState->OnEnter();


	//// Unsure if correct to do this here
	//if (myActiveTransition)
	//{
	//	myActiveTransition->GetTargetState()->OnExit();
	//}
	//
	//if (myActiveState)
	//{
	//	myActiveState->OnExit();
	//}

	// NOTE: We call OnStateEnter() in the StateBehaviour when we start the transition to the state
	// I am unsure if this is the correct decision atm
	// state->OnEnter();

	// When we start transitioning to another state
	// we consider the target state as the "active state"
	// meaning the previous state, is considered dead and is 
	// only there for the visual blending.
	// No events/callbacks are allowed to run anymore.
	// state->OnEnter();

	myActiveTransition = aTransition;
}

bool Engine::AnimationMachineLayer::ShouldDoTransition(Transition& aTransition, AnimationController& aController) const
{
	ZoneNamedN(scope1, "AnimationMachineLayer::ShouldDoTransition", true);

	if (myActiveTransition)
	{
		// We are not allowed to interrupt a transition made from AnyState
		// Why? 
		// Example:
		// CCTPrototype is doing a combo attack
		// We trigger a FireExplosion transition from AnyState
		// That FireExplosition transition has a fade of 0.5 seconds
		// During that transition fade, something else can interrupt it
		// and the actual transition would never occur
		if (myActiveTransition->IsAnyStateTransition())
		{
			return false;
		}
	}

	// If its able to interrupt, we don't bother waiting for the animation to finish
	// just set that shit immediately
	// Don't do transition if already started, why? 
	// Because it goes in an infinite loop
	if (aTransition.CanInterruptCurrentPlayingAnimation() &&
		aTransition.CanDoTransition() &&
		!aTransition.HasStartedTransitioning())
	{
		// INFO_PRINT("[%s] Interrupted Start: %s", myName.c_str(), aTransition.GetTargetState()->GetName().c_str());
		LOG_INFO(LogType::Animation) << "[" << aController.myName << "] Interrupt Start: " <<
			aTransition.GetTargetState()->GetName() <<
			" from Transition: " << aTransition.GetTransitionName();

		return true;
	}

	// If we have a state with no playable, we never want to make a transition based on elapsed time
	if (myActiveState && myActiveState->HasPlayable())
	{
		// This is the new "ExitTime"
		bool shouldTransition = myActiveState->GetElapsedTimeNormalized() >= aTransition.GetExitTimeNormalized();

		if (shouldTransition)
		{
			if (!aTransition.HasStartedTransitioning() && aTransition.CanDoTransition())
			{
				// INFO_PRINT("[%s] Fade Start: %s from Transition: %s", myName.c_str(), aTransition.GetTargetState()->GetName().c_str(), aTransition.GetTransitionName().c_str());
				LOG_INFO(LogType::Animation) << "[" << aController.myName << "] Normal Start: " <<
					aTransition.GetTargetState()->GetName() <<
					" from Transition: " << aTransition.GetTransitionName();

				return true;
			}
		}
	}

	return false;
}

void Engine::AnimationMachineLayer::SetStateByName(const std::string& aStateName)
{
	myActiveState = myStates.at(aStateName).get();
	LOG_INFO(LogType::Animation) << "Set State: " << aStateName;
}

void Engine::AnimationMachineLayer::SetActiveState(AnimationState* aAnimationState)
{
#ifdef _DEBUG
	bool existsInMyStates = false;

	for (const auto& [name, state] : myStates)
	{
		if (state.get() == aAnimationState)
		{
			existsInMyStates = true;
			break;
		}
	}

	assert(existsInMyStates);
#endif

	// assert(aAnimationState->GetName() != "Any State");

	myActiveState = aAnimationState;

	// LOG_INFO(LogType::Animation) << "Set State: " << aAnimationState->GetName();
}

void Engine::AnimationMachineLayer::RemoveActiveState()
{
	if (myActiveState)
	{
		LOG_INFO(LogType::Animation) << "Removed active state: " << myActiveState->GetName();
	}
	else
	{
		LOG_INFO(LogType::Animation) << "Removed active state: null";
	}

	myActiveState = nullptr;
}

void Engine::AnimationMachineLayer::AddState(Owned<AnimationState> aState)
{
	const std::string& stateName = aState->GetName();

	assert(myStates.find(stateName) == myStates.end() && "already exists");

	myStates[stateName] = std::move(aState);
}

Engine::AnimationState& Engine::AnimationMachineLayer::GetState(const std::string& aStateName)
{
	assert(myStates.find(aStateName) != myStates.end() && "does not exist");

	return *myStates.at(aStateName);
}

Engine::AnimationState* Engine::AnimationMachineLayer::TryGetState(const std::string& aStateName)
{
	if (myStates.find(aStateName) == myStates.end())
	{
		return nullptr;
	}

	return myStates[aStateName].get();
}

Engine::AnimationState* Engine::AnimationMachineLayer::GetActiveState()
{
	return myActiveState;
}

Engine::AnimationState& Engine::AnimationMachineLayer::GetAnyState()
{
	return *myAnyState;
}

void Engine::AnimationMachineLayer::Setup(const std::string& aInitialStateName)
{
	// SetStateByName(aInitialStateName);
	if (!aInitialStateName.empty())
	{
		myInitialState = &GetState(aInitialStateName);
	}

	// Fix up the references between the states and transitions
	// otherise, we'd refer to state using strings in the lookup table, slow
	for (const auto& [name, state] : myStates)
	{
		state->SetupReferences(*this);
	}

	// for (int i = static_cast<int>(myAnyStateTransitions.size()) - 1; i >= 0; --i)
	// {
	// 	auto& anyStateTransition = myAnyStateTransitions[i];
	// 
	// 	if (!anyStateTransition->SetupReferences(*this))
	// 	{
	// 		myAnyStateTransitions.erase(myAnyStateTransitions.begin() + i);
	// 	}
	// }

	// HACK: 
	// All animations needs to have the same bone, therefore 
	// we save ONE of the animations bones nodes
}

bool Engine::AnimationMachineLayer::HasInitialState() const
{
	return myInitialState != nullptr;
}

Engine::AnimationState& Engine::AnimationMachineLayer::GetInitialState()
{
	return *myInitialState;
}

const std::string& Engine::AnimationMachineLayer::GetName() const
{
	return myName;
}

Engine::AnimationMachineLayer::Type Engine::AnimationMachineLayer::GetType() const
{
	return myType;
}

const Engine::BlendMask& Engine::AnimationMachineLayer::GetBlendMask() const
{
	return myBlendMask;
}

float Engine::AnimationMachineLayer::GetWeight() const
{
	return myWeight;
}

void Engine::AnimationMachineLayer::SetWeight(float aWeight)
{
	myWeight = aWeight;
}
