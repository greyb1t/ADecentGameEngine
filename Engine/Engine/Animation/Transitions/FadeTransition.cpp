#include "pch.h"
#include "FadeTransition.h"
#include "..\AnimationController.h"
#include "..\State\AnimationState.h"
#include "..\Blending\BlendHelper.h"
#include "Engine/Animation/State/AnimationMachineLayer.h"

Engine::FadeTransition::FadeTransition()
{
}

void Engine::FadeTransition::InitFromJson(const nlohmann::json& aJson)
{
	Transition::InitFromJson(aJson);

	if (aJson.contains("FadeDurationSeconds"))
	{
		myDurationSeconds = aJson["FadeDurationSeconds"];
	}
	else
	{
		LOG_ERROR(LogType::Animation) << "Missing \"FadeDurationSeconds\" in " << myTransitionName;
	}
}

nlohmann::json Engine::FadeTransition::ToJson() const
{
	auto j = Transition::ToJson();

	j["FadeDurationSeconds"] = myDurationSeconds;

	return j;
}

float Engine::FadeTransition::GetDurationSeconds() const
{
	return myDurationSeconds;
}

void Engine::FadeTransition::Reset()
{
	Transition::Reset();

	myFadeTimerSeconds = 0.f;
}

Engine::TransitionStatus Engine::FadeTransition::Update(
	AnimationController& aController,
	const float aDeltaTime)
{
	//ZoneScopedN("FadeTransition::Update");

	//TransitionResult result;

	//// We want to start playing the animations that we want to fade to
	//myTargetState->Update(aController, aDeltaTime);

	//myFadeTimerSeconds += aDeltaTime;

	//const float fadePercent = CU::InverseLerp(
	//	0.f,
	//	GetDurationSeconds(),
	//	myFadeTimerSeconds);

	//const auto crossFadeLocalTransforms = myTargetState->CalculateInterpolatedTransforms();

	//result.myTransformResult = BlendHelper::Blend(aTransformResult, crossFadeLocalTransforms, fadePercent);

	//const bool isInstantFade = GetDurationSeconds() == 0.f;

	//if (fadePercent >= 1.f || isInstantFade)
	//{
	//	result.myTransitionState = TransitionResult::Finished;
	//}
	//else
	//{
	//	result.myTransitionState = TransitionResult::Ongoing;
	//}

	//return result;

	ZoneScopedN("FadeTransition::Update");

	// TransitionResult result;

	if (myTargetState->HasPlayable())
	{
		// We want to start playing the animations that we want to fade to
		myTargetState->Update(aDeltaTime);
	}
	else
	{
		LOG_ERROR(LogType::Animation) << "Target state in transition does not have a playable, doing transition instantly";

		return TransitionStatus::Finished;
	}

	myFadeTimerSeconds += aDeltaTime;

	const float fadePercent = C::InverseLerp(
		0.f,
		GetDurationSeconds(),
		myFadeTimerSeconds);

	const bool isInstantFade = GetDurationSeconds() == 0.f;

	if (fadePercent >= 1.f || isInstantFade)
	{
		return TransitionStatus::Finished;
	}
	else
	{
		return TransitionStatus::Ongoing;
	}
}

Engine::TransitionResult Engine::FadeTransition::CalculateTransforms(
	const AnimationTransformResult& aTransformResult)
{
	ZoneScopedN("FadeTransition::CalculateTransforms");

	TransitionResult result;

	const float fadePercent = C::InverseLerp(
		0.f,
		GetDurationSeconds(),
		myFadeTimerSeconds);

	// TODO: fix?
	if (!myTargetState->HasPlayable())
	{
		LOG_WARNING(LogType::Animation) << "Not blending transition to a target with no playable";
		result.myTransformResult = aTransformResult;
		return result;
	}

	const auto crossFadeLocalTransforms = myTargetState->CalculateInterpolatedTransforms();

	if (crossFadeLocalTransforms.myEntries.empty())
	{
		LOG_WARNING(LogType::Animation) << "A state is missing an animation";
		return result;
	}

	result.myTransformResult = BlendHelper::Blend(aTransformResult, crossFadeLocalTransforms, fadePercent);

	return result;
}

void Engine::FadeTransition::DoTransition(AnimationMachineLayer& aLayer)
{
	aLayer.SetActiveTransition(this);
}
