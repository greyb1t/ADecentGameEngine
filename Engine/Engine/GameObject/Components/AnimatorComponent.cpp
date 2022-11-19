#include "pch.h"
#include "AnimatorComponent.h"
#include "ModelComponent.h"
#include "Engine/Renderer\Model\ModelInstance.h"
#include "..\GameObject.h"
#include "Engine/ResourceManagement\Resources\ModelResource.h"
#include "..\Animation\AnimationController.h"
#include "..\Engine.h"
#include "Engine/Animation/AnimationPair.h"
#include "Engine/Reflection/Reflector.h"
#include "Engine/Renderer\GraphicsEngine.h"
#include "Engine\Renderer\Animation\AnimationClipLoader.h"
#include "Engine/ResourceManagement\Resources\AnimationClipResource.h"
#include "Engine/ResourceManagement\Resources\AnimationStateMachineResource.h"
#include "Engine\Animation\AnimationStateMachine.h"

#include "Engine/Animation/Transitions/Transition.h"
#include "Engine/Animation/Playable.h"

Engine::AnimatorComponent::AnimatorComponent(
	GameObject* aGameObject,
	ModelComponent& aSkinnedModelComponent,
	const std::string& aName)
	: Component(aGameObject),
	mySkinnedModelComponent(&aSkinnedModelComponent),
	myName(aName)
{
}

Engine::AnimatorComponent::AnimatorComponent(const AnimatorComponent& aOther)
	: Component(aOther),
	myJsonPath(aOther.myJsonPath),
	myAnimationResource(aOther.myAnimationResource),
	myName(aOther.myName)
{
	// NOTE(filip): we dont copy the machine or the controller because
	// too much work and not actually needed
	// only init them here?

	// if (!myJsonPath.empty())
	{
		LoadAnimationFromJson(myAnimationResource);
	}
}

Engine::AnimatorComponent::~AnimatorComponent()
{
#ifndef _RETAIL
	if (myAnimationResource)
	{
		myAnimationResource->RemoveObserver(this);
	}
#endif
}

void Engine::AnimatorComponent::Awake()
{
	mySkinnedModelComponent = myGameObject->GetComponent<ModelComponent>();

	myController.Init(myMachine, myName);

	if (mySkinnedModelComponent)
	{
		myController.SetModelInstance(mySkinnedModelComponent->GetModelInstance());
	}
}

void Engine::AnimatorComponent::InitFromJson(const std::string& aPath)
{
	myJsonPath = aPath;

	assert(!StringUtilities::EndsWith(aPath, ".json") && "extension is not .json anymore");

	// myMachine = MakeOwned<AnimationMachine>();

	/*
	GetEngine().GetFileWatcher().WatchFile(aPath,
		[this](const std::string& aPath)
	{
		LOG_INFO(LogType::Animation) << "Reloaded animation " << aPath;

		LoadAnimationFromJson(aPath);
	});
	*/

	LoadAnimationFromJson(aPath);

#ifndef _RETAIL
	if (myAnimationResource)
	{
		myAnimationResource->AddObserver(this);
	}
#endif
}

void Engine::AnimatorComponent::Execute(eEngineOrder aOrder)
{
	ZoneScopedN("AnimatorComponent::Execute");

#if 0
	auto& transform = myGameObject->GetTransform();

	if (mySkinnedModelComponent == nullptr ||
		!mySkinnedModelComponent->GetModelInstance().GetModel())
	{
		return;
	}

	if (!myAnimationResource || !myAnimationResource->IsValid())
	{
		return;
	}

	auto& skinnedModel = mySkinnedModelComponent->GetModelInstance().GetModel();

	if (skinnedModel->IsValid() && myController.myModelInstance != nullptr)
	{
		myController.Update(Time::DeltaTime);

		const auto rootMotionDelta = myController.GetRootMotionDelta();

		if (rootMotionDelta.myTranslation.Length() > 0.f)
		{
			//LOG_INFO(LogType::Simon) << rootMotionDelta.myTranslation.Length();

			const auto rootMotionDelta2 = myController.GetRootMotionDelta();

			int test = 0;
		}

		// We also need to rotate the translation to ensure that the rootmotion delta translation
		// is in the correct direction
		myGameObject->GetTransform().Move(rootMotionDelta.myTranslation * myGameObject->GetTransform().GetRotation());
	}
#endif // 0
}

void Engine::AnimatorComponent::ExecuteParallelized(eEngineOrder aOrder)
{
	START_TIMER(timer);

	ZoneNamedN(zone1, "AnimatorComponent::ExecuteParallelized", true);
	ZoneTextV(zone1, myController.myModelInstance->GetModel()->GetPath().c_str(), myController.myModelInstance->GetModel()->GetPath().size());

#if 1
	auto& transform = myGameObject->GetTransform();

	if (mySkinnedModelComponent == nullptr ||
		!mySkinnedModelComponent->GetModelInstance().GetModel())
	{
		return;
	}

	if (!myAnimationResource || !myAnimationResource->IsValid())
	{
		return;
	}

	auto& skinnedModel = mySkinnedModelComponent->GetModelInstance().GetModel();

	if (skinnedModel->IsValid() && myController.myModelInstance != nullptr)
	{
		myController.Update(Time::DeltaTime);

		const auto rootMotionDelta = myController.GetRootMotionDelta();

		if (rootMotionDelta.myTranslation.Length() > 0.f)
		{
			//LOG_INFO(LogType::Simon) << rootMotionDelta.myTranslation.Length();

			const auto rootMotionDelta2 = myController.GetRootMotionDelta();

			int test = 0;
		}

		if (GameObject* g = myRootMotionTarget.Get())
		{
			// We also need to rotate the translation to ensure that the rootmotion delta translation
		// is in the correct direction
			g->GetTransform().Move(rootMotionDelta.myTranslation * g->GetTransform().GetRotation());
		}
		else
		{
			// We also need to rotate the translation to ensure that the rootmotion delta translation
			// is in the correct direction
			myGameObject->GetTransform().Move(rootMotionDelta.myTranslation * myGameObject->GetTransform().GetRotation());

		}

	}
#endif

	float val = END_TIMER_GET_RESULT_MS(timer);
}

void Engine::AnimatorComponent::PostExecute()
{
	// Since we execute the above stuff parallellized
	// we must dispatch the events in main thread to avoid
	// problems with threading
	myController.CallCallbacks();
}

Engine::AnimationController& Engine::AnimatorComponent::GetController()
{
	return myController;
}

void Engine::AnimatorComponent::ReloadAnimation()
{
	LoadAnimationFromJson(myJsonPath);
}

void Engine::AnimatorComponent::OnResourceReloaded()
{
	ReloadAnimation();
}

void Engine::AnimatorComponent::Reflect(Reflector& aReflector)
{
	Component::Reflect(aReflector);

	AnimationPair pair(myAnimationResource);

	if (myMachine.IsInitialized() && HasStarted())
	{
		pair.myMachineInstance = &myMachine;
	}

	const auto animationResult = aReflector.Reflect(pair, "Animation");

	if (animationResult & ReflectorResult_Changed)
	{
		LoadAnimationFromJson(myAnimationResource);
	}

	myController.Reflect(aReflector);

	aReflector.Reflect(myRootMotionTarget, "Root Motion GameObject");
}

void Engine::AnimatorComponent::LoadAnimationFromJson(const std::string& aPath)
{
	myAnimationResource = GetEngine().GetResourceManager().CreateRef<AnimationStateMachineResource>(aPath);

	assert(myAnimationResource->GetState() == ResourceState::Loaded && "must be loaded");

	// myAnimationResource->Load();

	LoadAnimationFromJson(myAnimationResource);
}

void Engine::AnimatorComponent::LoadAnimationFromJson(const AnimationMachineRef& aAnimation)
{
	if (!myAnimationResource || !myAnimationResource->IsValid())
	{
		return;
	}

	//if (!myMachine.ReInitFromJson(myAnimationResource->Get().GetJson()))
	//{
	//	LOG_ERROR(LogType::Animation) << "Failed to create animations for " << myAnimationResource->GetPath();
	//	return;
	//}

	//myMachine = { };
	if (!myMachine.InitFromJson2(myAnimationResource->Get().GetJson()))
	{
		LOG_ERROR(LogType::Animation) << "Failed to create animations for " << myAnimationResource->GetPath();
		return;
	}

	//if (mySkinnedModelComponent)
	{
		//myController = { };
		myController.Init(myMachine, myName);
		//myController.SetModelInstance(mySkinnedModelComponent->GetModelInstance());
	}
}

bool Engine::AnimatorComponent::HasMachine() const
{
	return myAnimationResource && myAnimationResource->IsValid();
}