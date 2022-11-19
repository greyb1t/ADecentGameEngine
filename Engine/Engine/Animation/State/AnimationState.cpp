#include "pch.h"
#include "AnimationState.h"
#include "../Blending/BlendTree.h"
#include "../Playable.h"
#include "../Transitions/FadeTransition.h"
#include "../Playable.h"
#include "../AnimationStateBehaviour.h"
#include "../AnimationController.h"
#include "Engine/Animation/NormalAnimation.h"
#include "Engine/ResourceManagement/ResourceManager.h"
#include "Engine/ResourceManagement/Resources/AnimationClipResource.h"
#include "Engine/Animation/Blending/BlendTree1D.h"
#include "Engine/Animation/Blending/BlendTree2D.h"

Engine::AnimationState::AnimationState(
	const std::string& aStateName,
	Owned<Playable> aHasPose)
	: myName(aStateName),
	myPlayable(std::move(aHasPose))
{

}

Engine::AnimationState::~AnimationState()
{
}

float Engine::AnimationState::GetDurationSeconds() const
{
	if (myPlayable)
	{
		return myPlayable->GetDurationSeconds();
	}
	return 0.0f;
}

float Engine::AnimationState::GetTimeScale() const
{
	return myTimeScale;
}

void Engine::AnimationState::SetTimeScale(const float aTimeScale)
{
	myTimeScale = aTimeScale;
}

nlohmann::json Engine::AnimationState::ToJson() const
{
	nlohmann::json j;

	j["Node"] = AnimationNodeEditorWindow::Node::ToJson();

	j["StateName"] = myName;

	auto transitionsJsonArray = nlohmann::json::array();

	for (const auto& transition : myTransitions)
	{
		transitionsJsonArray.push_back(transition->ToJson());
	}

	j["Transitions"] = transitionsJsonArray;

	if (myPlayable)
	{
		j["Playable"] = myPlayable->ToJson();
	}

	j["StayWhenFinished"] = myStayWhenFinished;

	return j;
}

void Engine::AnimationState::InitFromJson2(const nlohmann::json& aJson, AnimationMachine& aMachine)
{
	AnimationNodeEditorWindow::Node::InitFromJson2(aJson["Node"]);

	myName = aJson["StateName"];

	for (const auto& transitionJson : aJson["Transitions"])
	{
		auto transition = MakeOwned<FadeTransition>();

		transition->InitFromJson(transitionJson);

		if (transition->HasTrigger())
		{
			transition->SetTriggerPtr(
				aMachine.GetTriggers().at(transition->GetTriggerName()).get());
		}

		for (auto& condGroup : transition->GetConditionGroups())
		{
			for (auto& cond : condGroup.GetConditions())
			{
				switch (cond.GetValueType())
				{
				case ConditionValueType::Bool:
					cond.SetValuePointer(aMachine.GetBools().at(cond.GetValueName()).get());
					break;
				case ConditionValueType::Int:
					cond.SetValuePointer(aMachine.GetInts().at(cond.GetValueName()).get());
					break;
				case ConditionValueType::Float:
					cond.SetValuePointer(aMachine.GetFloats().at(cond.GetValueName()).get());
					break;
				default:
					break;
				}
			}
		}

		AddTransition(std::move(transition));
	}

	if (aJson.contains("Playable"))
	{
		// TODO: turn into an enum
		const std::string playableType = aJson["Playable"]["Type"];

		if (playableType == "Animation")
		{
			myPlayable = MakeOwned<NormalAnimation>();
			myPlayable->InitFromJson2(aJson["Playable"], aMachine);
		}
		else if (playableType == "BlendTree1D")
		{
			myPlayable = MakeOwned<BlendTree1D>();
			myPlayable->InitFromJson2(aJson["Playable"], aMachine);
		}
		else if (playableType == "BlendTree2D")
		{
			myPlayable = MakeOwned<BlendTree2D>();
			myPlayable->InitFromJson2(aJson["Playable"], aMachine);
		}
		else
		{
			assert(false);
		}
	}

	if (aJson.contains("StayWhenFinished"))
	{
		myStayWhenFinished = aJson["StayWhenFinished"];
	}
}

Engine::AnimationTransformResult Engine::AnimationState::CalculateInterpolatedTransforms()
{
	return myPlayable->CalculateInterpolatedTransforms();
}

void Engine::AnimationState::AddTransition(Owned<Transition> aTransition)
{
	myTransitions.push_back(std::move(aTransition));
}

bool Engine::AnimationState::HasFinished() const
{
	return myPlayable->HasFinished();
}

float Engine::AnimationState::GetElapsedTimeNormalized() const
{
	return myPlayable->GetElapsedTimeNormalized();
}

void Engine::AnimationState::ResetPlayable()
{
	if (myPlayable)
	{
		myPlayable->Reset();
	}
}

void Engine::AnimationState::ResetState()
{
	myHasEntered = false;
	myHasExited = false;
}

void Engine::AnimationState::Update(const float aDeltaTime)
{
	ZoneNamedN(zone1, "AnimationState::Update", true);

	if (!myPlayable->CanPlay())
	{
		return;
	}

	const bool isActive = IsActive();

	if (isActive && myStateBehaviour)
	{
		// TODO: Removed because not used, if adding again
		// must call it after we have updated the animations
		// parallelsized. TO AVOID THREADING ISSUES
		// myStateBehaviour->OnAnimationStateUpdate();
	}

	myPlayable->Update(*this, aDeltaTime * myTimeScale);

	if (isActive)
	{
		if (myPlayable->HasFinished())
		{
			OnExit();
		}

		// Only if the state is active, then we call the events
		for (const AnimationEvent* event : myPlayable->GetEventsExecutedThisFrame())
		{
			myController->AddExecutedEvent(event);
		}
	}
}

std::vector<Owned<Engine::Transition>>& Engine::AnimationState::GetTransitions()
{
	return myTransitions;
}

const std::string& Engine::AnimationState::GetName() const
{
	return myName;
}

void Engine::AnimationState::SetupReferences(AnimationMachineLayer& aLayer)
{
	for (int i = static_cast<int>(myTransitions.size()) - 1; i >= 0; --i)
	{
		if (!myTransitions[i]->SetupReferences(aLayer))
		{
			// Remove the transition
			myTransitions.erase(myTransitions.begin() + i);
		}
	}
}

bool Engine::AnimationState::HasPlayable() const
{
	return myPlayable != nullptr;
}

Engine::Playable& Engine::AnimationState::GetPlayable()
{
	return *myPlayable;
}

void Engine::AnimationState::SetStateBehaviour(AnimationStateBehaviour* aStateBehaviour)
{
	myStateBehaviour = aStateBehaviour;
}

void Engine::AnimationState::OnEnter()
{
	assert(!myHasEntered && "Must only be called once");

	myHasEntered = true;

	std::string playableName = "No playable";

	if (myPlayable)
	{
		playableName = myPlayable->GetName();
	}

	LOG_INFO(LogType::Animation) << "OnEnter: " << playableName;

	if (myStateBehaviour)
	{
		//myStateBehaviour->OnAnimationStateEnter();

		myController->AddCallbackToRunThisFrame(
			[this]()
			{
				myStateBehaviour->OnAnimationStateEnter();
			});
	}
}

void Engine::AnimationState::OnExit()
{
	assert(myHasEntered && "Must have entered, to be able to exit");
	// assert(!myHasExited && "Must only be called once");

	if (myHasExited)
	{
		return;
	}

	myHasExited = true;

	std::string playableName = "No playable";

	if (myPlayable)
	{
		playableName = myPlayable->GetName();
	}

	LOG_INFO(LogType::Animation) << "OnExit: " << playableName;

	if (myStateBehaviour)
	{
		//myStateBehaviour->OnAnimationStateExit();

		myController->AddCallbackToRunThisFrame(
			[this]()
			{
				myStateBehaviour->OnAnimationStateExit();
			});
	}
}

bool Engine::AnimationState::IsActive() const
{
	return myHasEntered && !myHasExited;
}

void Engine::AnimationState::AddExecutedEvent(const AnimationEvent* aEvent)
{
	myExecutedEventsThisFrame.push_back(aEvent);
}

void Engine::AnimationState::RemoveTriggerUsage(const std::string& aTriggerName)
{
	for (const auto& transition : myTransitions)
	{
		transition->RemoveTriggerUsage(aTriggerName);
	}

	if (myPlayable)
	{
		myPlayable->RemoveTriggerUsage(aTriggerName);
	}
}

void Engine::AnimationState::RemoveBoolUsage(const std::string& aConditionName)
{
	for (const auto& transition : myTransitions)
	{
		transition->RemoveBoolUsage(aConditionName);
	}

	if (myPlayable)
	{
		myPlayable->RemoveConditionUsage(aConditionName);
	}
}

void Engine::AnimationState::RemoveFloatUsage(const std::string& aFloatName)
{
	for (const auto& transition : myTransitions)
	{
		transition->RemoveFloatUsage(aFloatName);
	}

	if (myPlayable)
	{
		myPlayable->RemoveFloatUsage(aFloatName);
	}
}

void Engine::AnimationState::RemoveVec2Usage(const std::string& aVec2Name)
{
	for (const auto& transition : myTransitions)
	{
		transition->RemoveVec2Usage(aVec2Name);
	}

	if (myPlayable)
	{
		myPlayable->RemoveVec2Usage(aVec2Name);
	}
}

void Engine::AnimationState::RemoveIntUsage(const std::string& aIntName)
{
	for (const auto& transition : myTransitions)
	{
		transition->RemoveIntUsage(aIntName);
	}

	if (myPlayable)
	{
		myPlayable->RemoveIntUsage(aIntName);
	}
}

void Engine::AnimationState::SetController(AnimationController& aController)
{
	myController = &aController;
}

bool Engine::AnimationState::StayWhenFinished() const
{
	return myStayWhenFinished;
}
