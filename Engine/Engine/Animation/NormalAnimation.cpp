#include "pch.h"
#include "NormalAnimation.h"
#include "Engine/Renderer\Animation\AnimationClip.h"
#include "..\Engine.h"
#include "Engine/Renderer\GraphicsEngine.h"
#include "Engine/Renderer\Animation\AnimationClipLoader.h"
#include "AnimationController.h"
#include "Engine/ResourceManagement\ResourceManager.h"
#include "Engine/ResourceManagement\Resources\AnimationClipResource.h"
#include "Visitor/PlayableVisitor.h"

Engine::NormalAnimation::NormalAnimation()
{
}

nlohmann::json Engine::NormalAnimation::ToJson() const
{
	nlohmann::json j;

	if (myClip && myClip->IsValid())
	{
		j["ClipPath"] = myClip->GetPath();
	}
	j["PlayableName"] = myName;
	j["Type"] = "Animation";

	j["IsLooping"] = myIsLooping;
	j["IsRootMotion"] = myIsRootMotion;
	j["TimeScale"] = myTimeScale;

	nlohmann::json eventsArray;

	for (const auto& ev : myEvents)
	{
		eventsArray.push_back(ev.ToJson());
	}

	j["Events"] = eventsArray;

	return j;
}

void Engine::NormalAnimation::InitFromJson2(const nlohmann::json& aJson, AnimationMachine& aMachine)
{
	if (aJson.contains("ClipPath"))
	{
		const std::string clipPath = aJson["ClipPath"];

		myClip = GetEngine().GetResourceManager().CreateRef<AnimationClipResource>(clipPath);

		// TODO: Remove this when I re-saved all .animation to hold its animation clips list
		myClip->Load();
		//myClip->RequestLoading();

		aMachine.myAnimationClips.push_back(myClip);

		// YIKES
		if (!aMachine.GetFirstAnimationClip())
		{
			aMachine.myFirstAnimationClip = GResourceManager->CreateRef<AnimationClipResource>(clipPath);
		}
	}

	myName = aJson["PlayableName"];

	myIsLooping = aJson["IsLooping"];
	myIsRootMotion = aJson["IsRootMotion"];
	myTimeScale = aJson["TimeScale"];


	if (aJson.contains("Events"))
	{
		myEvents.reserve(aJson["Events"].size());

		for (const auto& eventJson : aJson["Events"])
		{
			AnimationEvent event;

			if (!event.InitFromJson(eventJson))
			{
				continue;
			}

			myEvents.push_back(event);
			myEventsNotExecuted.push_back(&myEvents.back());
		}
	}
}

bool Engine::NormalAnimation::InitFromJson(const nlohmann::json& aJson)
{
	const std::string clipPath = aJson["ClipPath"];

	myClip = GetEngine().GetResourceManager().CreateRef<AnimationClipResource>(clipPath);

	// TODO: Preload animations properly when done
	//assert(myClip->GetState() == ResourceState::Loaded && "must be loaded");
	myClip->Load();

	// myClip->RequestLoading();

	// if (!myClip->IsValid())
	// {
	// 	return false;
	// }

	if (aJson.contains("PlayableName"))
	{
		myName = aJson["PlayableName"];
	}

	myIsLooping = aJson["IsLooping"];
	myIsRootMotion = aJson["IsRootMotion"];

	if (aJson.contains("TimeScale"))
	{
		myTimeScale = aJson["TimeScale"];
	}

	if (aJson.contains("Events"))
	{
		myEvents.reserve(aJson["Events"].size());

		for (const auto& eventJson : aJson["Events"])
		{
			AnimationEvent event;

			if (!event.InitFromJson(eventJson))
			{
				continue;
			}

			myEvents.push_back(event);
			myEventsNotExecuted.push_back(&myEvents.back());
		}
	}

	return true;
}

Engine::AnimationTransformResult Engine::NormalAnimation::CalculateInterpolatedTransforms()
{
	ZoneScopedN("NormalAnimation::CalculateInterpolatedTransforms");

	if (!CanPlay())
	{
		return {};
	}

	AnimationTransformResult result;

	const auto& boneNodes = myClip->Get().GetBoneNodes();
	const auto& toBoneTrackIndex = myClip->Get().GetToBoneTrackIndex();
	const auto& tracks = myClip->Get().GetTracks();

	for (const auto& boneNode : boneNodes)
	{
		Vec3f scale = boneNode.myScale;
		Quatf rotation = boneNode.myRotation;
		Vec3f translation = boneNode.myTranslation;

		if (scale.x > 1.f || scale.y > 1.f || scale.z > 1.f)
		{
			int test = 0;
		}

		// const int trackIndex = toBoneTrackIndex.at(boneNode.myName);
		const int trackIndex = boneNode.myBoneNameToTrackIndex;
		const Track* boneAnim = nullptr;

		if (trackIndex != -1)
		{
			boneAnim = &tracks[trackIndex];
		}

		if (boneAnim)
		{
			scale = boneAnim->CalcInterpolatedScaling(myAnimTimeInTicks);
			rotation = boneAnim->CalcInterpolatedRotation(myAnimTimeInTicks);
			translation = boneAnim->CalcInterpolatedPosition(myAnimTimeInTicks);
		}

		if (scale.x > 1.f || scale.y > 1.f || scale.z > 1.f)
		{
			int test = 0;
		}

		AnimationTransform entry;
		entry.myScale = scale;
		entry.myRotation = rotation;
		entry.myTranslation = translation;

		result.AddEntry(entry);
	}

	{
		myRootMotionPrevious = myRootMotion;

		myRootMotion.myScale = result.myEntries[1].myScale;
		myRootMotion.myRotation = result.myEntries[1].myRotation;
		myRootMotion.myTranslation = result.myEntries[1].myTranslation;
	}

	return result;
}

Engine::AnimationTransform Engine::NormalAnimation::GetRootMotionDelta() const
{
	AnimationTransform rootMotionDelta;
	{
		rootMotionDelta.myScale = myRootMotion.myScale - myRootMotionPrevious.myScale;

		// TODO: Is this Inverse correct? or should the other one be Inverse?
		rootMotionDelta.myRotation = myRootMotion.myRotation * myRootMotionPrevious.myRotation.GetInverse();

		rootMotionDelta.myTranslation = myRootMotion.myTranslation - myRootMotionPrevious.myTranslation;
	}

	return rootMotionDelta;
}

void Engine::NormalAnimation::Update(AnimationState& aState, const float aDeltaTime)
{
	ZoneScopedN("NormalAnimation::Update");

	myEventsExecutedThisFrame.clear();

	if (!myClip || !myClip->IsValid())
	{
		LOG_WARNING(LogType::Animation) << "Cannot play: " << myName << ", because clip not valid";
		return;
	}

	for (int i = static_cast<int>(myEventsNotExecuted.size()) - 1; i >= 0; --i)
	{
		AnimationEvent* event = myEventsNotExecuted[i];

		if (event->WasExecuted(*this))
		{
			LOG_INFO(LogType::Animation) << "Event: " << event->GetName() << " executed";

			// Add to for the callback to be called in the controller
			myEventsExecutedThisFrame.push_back(event);

			// Remove the callback from this vector
			myEventsNotExecuted.erase(myEventsNotExecuted.begin() + i);
		}
	}

	if (myAnimTimeInTicks >= myClip->Get().GetDurationInTicks())
	{
		if (myIsLooping)
		{
			Reset();
		}
		else
		{
			// If not looping, freeze the animation at the last frame
			myAnimTimeInTicks = myClip->Get().GetDurationInTicks();
		}
	}

	if (myIsLooping)
	{
		myAnimTimeInTicks += aDeltaTime * myTimeScale * myClip->Get().GetTicksPerSecond();
	}
	else
	{
		if (!HasFinished())
		{
			myAnimTimeInTicks += aDeltaTime * myTimeScale * myClip->Get().GetTicksPerSecond();
		}
	}

	// Clamp it to not spill over the max duration
	myAnimTimeInTicks = std::min(myAnimTimeInTicks, myClip->Get().GetDurationInTicks());
}

void Engine::NormalAnimation::SetTimeScale(const float aTimeScale)
{
	myTimeScale = aTimeScale;
}

float Engine::NormalAnimation::GetTimeScale() const
{
	return myTimeScale;
}

void Engine::NormalAnimation::SetIsLooping(const bool aIsLooping)
{
	myIsLooping = aIsLooping;
}

bool Engine::NormalAnimation::IsLooping() const
{
	return myIsLooping;
}

bool Engine::NormalAnimation::IsRootMotion() const
{
	return myIsRootMotion;
}

void Engine::NormalAnimation::SetIsRootMotion(const bool aIsRootMotion)
{
	myIsRootMotion = aIsRootMotion;
}

bool Engine::NormalAnimation::HasFinished() const
{
	if (myIsLooping)
	{
		return false;
	}

	return myAnimTimeInTicks >= myClip->Get().GetDurationInTicks();
}

void Engine::NormalAnimation::Reset()
{
	myAnimTimeInTicks = 0.f;

	// myFinished = false;

	// When we reset the animation, the prev root motion result has to be reset as well
	myRootMotion = {};
	myRootMotionPrevious = {};

	myEventsNotExecuted.clear();

	for (auto& event : myEvents)
	{
		myEventsNotExecuted.push_back(&event);
	}
}

float Engine::NormalAnimation::GetElapsedTimeNormalized() const
{
	if (!myClip || !myClip->IsValid())
	{
		return 0.f;
	}

	return C::InverseLerp(0.f, myClip->Get().GetDurationInTicks(), myAnimTimeInTicks);
}

float Engine::NormalAnimation::GetCurrentFrame() const
{
	return myAnimTimeInTicks;
}

float Engine::NormalAnimation::GetDurationSeconds() const
{
	return myClip->Get().GetDurationSeconds();
}

void Engine::NormalAnimation::SetElapsedTimeNormalized(const float aTimeNormalized)
{
	assert(aTimeNormalized >= 0.f && aTimeNormalized <= 1.f);
	myAnimTimeInTicks = C::Lerp(0.f, myClip->Get().GetDurationInTicks(), aTimeNormalized);
}

const std::string& Engine::NormalAnimation::GetName() const
{
	return myName;
}

std::vector<const Engine::AnimationEvent*> Engine::NormalAnimation::GetEventsExecutedThisFrame()
{
	return myEventsExecutedThisFrame;
}

bool Engine::NormalAnimation::CanPlay() const
{
	return myClip && myClip->IsValid();
}

void Engine::NormalAnimation::Accept(PlayableVisitor& aVisitor)
{
	aVisitor.Visit(*this);
}
