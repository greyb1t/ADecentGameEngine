#include "pch.h"
#include "AnimationController.h"
#include "Engine/ResourceManagement\Resources\ModelResource.h"
#include "Engine/Renderer\Animation\AnimationClip.h"
#include "State\AnimationState.h"
#include "Blending\BlendHelper.h"

#include "..\..\Engine\Engine.h"
#include "Transitions/FadeTransition.h"
#include "Engine/Renderer\Model\ModelInstance.h"
#include "AnimationEvent.h"
#include "Playable.h"
#include "Engine/ResourceManagement\Resources\AnimationClipResource.h"
#include "Engine/Renderer\Animation\AssimpUtils.h"
#include "Engine/Renderer\Model\Model.h"
#include "State/AnimationMachineLayer.h"
#include "Engine/Reflection/Reflector.h"
#include "Engine/Renderer/Shaders/ShaderConstants.h"

Engine::AnimationController::AnimationController()
{
}

Engine::AnimationController::~AnimationController()
{
}

bool Engine::AnimationController::Init(
	AnimationMachine& aMachine,
	const std::string& aName)
{
	myMachine = &aMachine;

	myName = aName;

	myMachine->ResetToInitialState();

	myMachine->SetController(*this);

	return true;
}

void Engine::AnimationController::SetModelInstance(ModelInstance& aModelInstance)
{
	myModelInstance = &aModelInstance;
}

void Engine::AnimationController::Update(const float aDeltaTime)
{
	ZoneScopedN("AnimationController::Update");

	const auto& firstClip = myMachine->GetFirstAnimationClip();

	if (!firstClip || !firstClip->IsValid())
	{
		return;
	}

	const auto localTrans = myMachine->Update(aDeltaTime * myTimeScale, *this);

	for (auto& entry : localTrans.myEntries)
	{
		if (entry.myScale.x > 1.f)
		{
			int test = 0;
		}
	}

	if (localTrans.myEntries.empty())
	{
		LOG_WARNING(LogType::Animation) << "No animation results";
		return;
	}

	const auto& boneNodes = firstClip->Get().GetBoneNodes();

	const bool sameSkeleton = boneNodes.size() == localTrans.myEntries.size();

	if (sameSkeleton)
	{
		if (myModelInstance->PassedRenderCheck())
		{
			myFinalBoneTransforms = {};
			myFinalBoneTransformsWorldSpace = {};

			std::vector<Mat4f> localTransMatrices;

			for (const auto& entry : localTrans.myEntries)
			{
				const auto scaleMat = Mat4f::CreateScale(entry.myScale);
				const auto rotationMat = entry.myRotation.ToMatrix();
				const auto translationMat = Mat4f::CreateTranslation(entry.myTranslation);

				const Mat4f mat = scaleMat * rotationMat * translationMat;

				localTransMatrices.push_back(mat);
			}

			CalculateFinalTransforms(
				boneNodes,
				localTransMatrices,
				myFinalBoneTransforms,
				myFinalBoneTransformsWorldSpace);
		}
	}

	myModelInstance->SetFinalAnimTransforms(myFinalBoneTransforms);

	// Reset the triggers at end of frame ONCE
	for (auto& [name, triggerValue] : myMachine->myTriggers)
	{
		*triggerValue = false;
	}
}

void Engine::AnimationController::CallCallbacks()
{
	// Call the state callbacks
	for (const auto& callback : myCallbacksToRunThisFrame)
	{
		callback();
	}

	myCallbacksToRunThisFrame.clear();

	// Call all events that was triggered
	for (const auto& executedEvent : myExecutedEventsThisFrame)
	{
		auto findResult = myEventCallbacks.find(executedEvent->GetName());

		if (findResult != myEventCallbacks.end())
		{
			LOG_INFO(LogType::Animation) << "CCTPrototype event callback: " << executedEvent->GetName();
			findResult->second();
		}
	}

	myExecutedEventsThisFrame.clear();
}

Engine::AnimationTransform Engine::AnimationController::GetRootMotionDelta() const
{
	AnimationTransform result;

	for (const auto& layer : myMachine->GetLayers())
	{
		if (layer->GetActiveState() == nullptr)
		{
			continue;
		}

		if (!layer->GetActiveState()->HasPlayable())
		{
			continue;
		}

		const auto& playable = layer->GetActiveState()->GetPlayable();

		if (playable.IsRootMotion())
		{
			const auto delta = playable.GetRootMotionDelta();
			result.myRotation = result.myRotation * delta.myRotation;
			result.myScale *= delta.myScale;
			result.myTranslation += delta.myTranslation;
		}
	}

	return result;
}

void Engine::AnimationController::Trigger(const std::string& aTriggerName)
{
	myMachine->Trigger(aTriggerName);
}

bool Engine::AnimationController::HasTrigger(const std::string& aTriggerName) const
{
	return myMachine->HasTrigger(aTriggerName);
}

bool Engine::AnimationController::HasBool(const std::string& aBoolName) const
{
	return myMachine->HasBool(aBoolName);
}

void Engine::AnimationController::SetBool(const std::string& aBoolName, const bool aValue)
{
	myMachine->SetBool(aBoolName, aValue);
}

bool Engine::AnimationController::GetBool(const std::string& aBoolName) const
{
	return myMachine->GetBool(aBoolName);
}

void Engine::AnimationController::SetFloat(const std::string& aFloatName, const float aValue)
{
	return myMachine->SetFloat(aFloatName, aValue);
}

float Engine::AnimationController::GetFloat(const std::string& aFloatName) const
{
	return myMachine->GetFloat(aFloatName);
}

void Engine::AnimationController::SetVec2(const std::string& aVecName, const Vec2f& aValue)
{
	return myMachine->SetVec2(aVecName, aValue);
}

const Vec2f& Engine::AnimationController::GetVec2(const std::string& aVecName) const
{
	return myMachine->GetVec2(aVecName);
}

void Engine::AnimationController::SetInt(const std::string& anIntName, const int aValue)
{
	myMachine->SetInt(anIntName, aValue);
}

int Engine::AnimationController::GetInt(const std::string& anIntName)
{
	return myMachine->GetInt(anIntName);
}

void Engine::AnimationController::AddEventCallback(const std::string& aEventName, std::function<void()> aCallback)
{
	if (myEventCallbacks.find(aEventName) != myEventCallbacks.end())
	{
		LOG_ERROR(LogType::Animation) << "Callback already exists from the event, ask "
			"filip to add support for multiple callbacks if needed";
	}
	else
	{
		myEventCallbacks[aEventName] = aCallback;
	}
}

void Engine::AnimationController::AddStateBehaviour(const std::string& aLayerName, const std::string& aStateName, AnimationStateBehaviour* aStateBehaviour)
{
	assert(aStateBehaviour);

	if (auto layer = myMachine->FindLayer(aLayerName))
	{
		auto& state = layer->GetState(aStateName);
		state.SetStateBehaviour(aStateBehaviour);
	}
	else
	{
		LOG_ERROR(LogType::Animation) << "Cannot find Layer '" << aLayerName << "'";
	}
}

class StateBehaviourPlaceholder : public Engine::AnimationStateBehaviour
{
public:
	StateBehaviourPlaceholder(std::function<void()> aCallback, std::function<void()> aCallback2)
		: myCallback(aCallback),
		myCallback2(aCallback2)
	{
	}

	void OnAnimationStateExit() override
	{
		if (myCallback)
		{
			myCallback();
		}
	}

	void OnAnimationStateEnter() override
	{
		if (myCallback2)
		{
			myCallback2();
		}
	}

private:
	std::function<void()> myCallback;
	std::function<void()> myCallback2;
};

void Engine::AnimationController::AddStateOnExitCallback(
	const std::string& aLayerName,
	const std::string& aStateName,
	std::function<void()> aOnExit,
	std::function<void()> aOnEnter)
{
	auto behaviour = MakeOwned<StateBehaviourPlaceholder>(aOnExit, aOnEnter);

	AddStateBehaviour(aLayerName, aStateName, behaviour.get());

	myStateBehavioursForExitOnly[aStateName] = std::move(behaviour);
}

int Engine::AnimationController::GetBoneIndexFromName(const std::string& aBoneName) const
{
	if (myModelInstance == nullptr)
	{
		return -1;
	}

	const auto& boneToNameIndex = myModelInstance->GetModel()->Get().GetBoneNameToIndex();

	if (boneToNameIndex.find(aBoneName) != boneToNameIndex.end())
	{
		const unsigned int boneIndex = boneToNameIndex.at(aBoneName);

		return boneIndex;
	}

	return -1;
}

Mat4f Engine::AnimationController::GetBoneTransformWorld(const int aBoneIndex)
{
	if (aBoneIndex == -1)
	{
		LOG_ERROR(LogType::Animation) << "Bone index is -1";
		return myFinalBoneTransformsWorldSpace[0] * myModelInstance->GetTransformMatrix();
	}

	return myFinalBoneTransformsWorldSpace[aBoneIndex] * myModelInstance->GetTransformMatrix();
}


Mat4f Engine::AnimationController::GetBoneTransformWorld(const std::string& aBoneName)
{
	return myFinalBoneTransformsWorldSpace[GetBoneIndexFromName(aBoneName)] * myModelInstance->GetTransformMatrix();
}


void Engine::AnimationController::SetLayerWeight(const std::string& aLayerName, const float aWeight)
{
	auto layer = myMachine->FindLayer(aLayerName);

	if (layer)
	{
		layer->SetWeight(aWeight);
	}
	else
	{
		LOG_ERROR(LogType::Animation) << "Cannot find layer: " << aLayerName;
	}
}

const float Engine::AnimationController::GetLayerWeight(const std::string& aLayerName) const
{
	auto layer = myMachine->FindLayer(aLayerName);

	if (layer)
	{
		return layer->GetWeight();
	}
	else
	{
		LOG_ERROR(LogType::Animation) << "Cannot find layer: " << aLayerName;
	}

	return -1.0f;
}

Engine::AnimationState* Engine::AnimationController::FindState(
	const std::string& aLayerName,
	const std::string& aStateName) const
{
	auto layer = myMachine->FindLayer(aLayerName);

	if (layer)
	{
		return layer->TryGetState(aStateName);
	}
	else
	{
		LOG_ERROR(LogType::Animation) << "Cannot find layer: " << aLayerName;
	}

	return nullptr;
}

float Engine::AnimationController::GetTimeScale() const
{
	return myTimeScale;
}

void Engine::AnimationController::SetTimeScale(const float aTimeScale)
{
	myTimeScale = aTimeScale;
}

void Engine::AnimationController::SetLayerTimeScale(const std::string& aLayerName, float aTimeScale)
{
	if (auto layer = myMachine->FindLayer(aLayerName))
	{
		layer->SetTimeScale(aTimeScale);
	}
	else
	{
		LOG_ERROR(LogType::Animation) << "Unable to find layer '" << aLayerName << "'";
	}
}

void Engine::AnimationController::CalculateFinalTransforms(
	const std::vector<OurNode>& aBoneNodes,
	const std::vector<Mat4f>& aLocalTransforms,
	std::array<Mat4f, ShaderConstants::MaxBoneCount>& aFinalTransformOffsetsOut,
	std::array<Mat4f, ShaderConstants::MaxBoneCount>& aFinalTransformWorldSpaceOut) const
{
	ZoneScopedN("AnimationController::CalculateFinalTransforms");

	const auto& model = myModelInstance->GetModel();

	const auto& boneNodes = /*aClip->GetBoneNodes()*/aBoneNodes;
	// const auto& boneNodes = model.GetModelData().myFbx->Get().GetBoneNodes();

	// std::vector<Mat4f> toRootTransforms;
	toRootTransforms.clear();
	if (toRootTransforms.size() != boneNodes.size())
	{
		toRootTransforms.resize(boneNodes.size());
	}

	// Root has no parent, therefore is its local transform
	toRootTransforms[0] = aLocalTransforms[0];

	Vec3f prevScale = aLocalTransforms[0].DecomposeScale();

	// Create the transform hierarchy of transform multiplications
	// We basically multiply each parents value with its previous multiplication
	for (int i = 1; i < boneNodes.size(); ++i)
	{

		const auto& localAnimTransform = aLocalTransforms[i];

		const int parentIndex = boneNodes[i].myParentIndex;
		const auto& parentToRoot = toRootTransforms[parentIndex];

		const auto& parentLocalAnimTransform = aLocalTransforms[parentIndex];

		Vec3f trans;
		Vec3f scale;
		Quatf rot;
		parentToRoot.Decompose(trans, rot, scale);

		//Vec3f tempScale = scale;
		//scale *= (1.f / prevScale);
		///*
		//scale = prevScale;
		// scale = Vec3f(1,1.f,1);
		//scale.x = 1.f;
		//scale.z = 1.f;
		//*/
		//prevScale = tempScale;
		//prevScale *= 1.f / scale;

		// this was the non-working solution to the non compensated scale issue
		const auto parentToRoot2 = Mat4f::CreateScale(scale * (1.f / parentLocalAnimTransform.DecomposeScale()))
			* rot.ToMatrix()
			* Mat4f::CreateTranslation(trans);

		const Mat4f toRoot = localAnimTransform * parentToRoot2;

		toRootTransforms[i] = toRoot;
	}

	// Finally get the final transforms by using the bone offsets
	for (int i = 0; i < boneNodes.size(); ++i)
	{
		const auto& node = boneNodes[i];

		const auto& boneNameToIndex = model->Get().GetBoneNameToIndex();
		const auto& boneOffsets = model->Get().GetBoneOffsets();

		//if (boneNameToIndex.find(node.myName) != boneNameToIndex.end())
		if (node.myBoneNameToIndex != -1)
		{
			//const unsigned int boneIndex = boneNameToIndex.at(node.myName);
			const unsigned int boneIndex = node.myBoneNameToIndex;

			// TODO: make this work
			const auto boneIndexCached = node.myBoneNameToIndex;

			const auto& globalInverseTransform =
				model->Get().GetGlobalInverseTransform();

			aFinalTransformOffsetsOut[boneIndex] =
				boneOffsets[boneIndex] *
				toRootTransforms[i] * globalInverseTransform;

			aFinalTransformWorldSpaceOut[boneIndex] = toRootTransforms[i];
		}
	}
}

void Engine::AnimationController::AddExecutedEvent(const AnimationEvent* aEvent)
{
	myExecutedEventsThisFrame.push_back(aEvent);
}

void Engine::AnimationController::AddCallbackToRunThisFrame(const std::function<void()>& aCallback)
{
	myCallbacksToRunThisFrame.push_back(aCallback);
}

Engine::AnimationMachine& Engine::AnimationController::GetMachine()
{
	return *myMachine;
}

void Engine::AnimationController::Reflect(Reflector& aReflector)
{
	aReflector.SetNextItemRange(0.f, 100.f);
	aReflector.SetNextItemSpeed(0.01f);
	aReflector.Reflect(myTimeScale, "Time Scale");
}

const Engine::AnimationMachine& Engine::AnimationController::GetAnimationMachine() const
{
	return *myMachine;
}
