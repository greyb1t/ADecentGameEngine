#include "pch.h"
#include "AnimationMachine.h"

#include "AnimationMachineLayer.h"
#include "../Engine.h"
#include "Engine/Renderer\GraphicsEngine.h"
#include "Engine/Renderer\Animation\AnimationClipLoader.h"
#include "..\NormalAnimation.h"
#include "..\Transitions\FadeTransition.h"
#include "Engine/Animation/Blending/BlendHelper.h"
#include "Engine/ResourceManagement/ResourceManager.h"
#include "Engine/ResourceManagement/Resources/AnimationClipResource.h"
#include "Engine/Animation/AnimationController.h"
#include "Engine/Renderer/Animation/AnimationClip.h"
#include "Engine/Renderer/Model/Model.h"
#include "Engine/Renderer/Model/ModelInstance.h"
#include "Engine/ResourceManagement/Resources/ModelResource.h"

Engine::AnimationMachine::AnimationMachine()
{
}

Engine::AnimationMachine::~AnimationMachine()
{
}

void Engine::AnimationMachine::ResetToInitialState()
{
	for (const auto& layer : myLayers)
	{
		layer->ResetToInitialState();
	}
}

void Engine::AnimationMachine::SetController(AnimationController& aController)
{
	for (const auto& layer : myLayers)
	{
		layer->SetController(aController);
	}
}

bool Engine::AnimationMachine::InitFromJson2(const nlohmann::json& aJson)
{
	myFloats.clear();
	myVec2s.clear();
	myInts.clear();
	myBools.clear();
	myTriggers.clear();

	myLayers.clear();

	myFirstAnimationClip = {};

	myIsInitialized = false;

	if (aJson.is_null())
	{
		return true;
	}

	if (aJson.contains("Triggers"))
	{
		for (const auto& triggerName : aJson["Triggers"])
		{
			myTriggers[triggerName] = MakeOwned<bool>();
		}
	}

	if (aJson.contains("Conditions"))
	{
		for (const auto& boolName : aJson["Conditions"])
		{
			myBools[boolName] = MakeOwned<bool>();
		}
	}

	if (aJson.contains("Floats"))
	{
		for (const auto& floatName : aJson["Floats"])
		{
			myFloats[floatName] = MakeOwned<float>();
		}
	}

	if (aJson.contains("Vec2s"))
	{
		for (const auto& vec2Name : aJson["Vec2s"])
		{
			myVec2s[vec2Name] = MakeOwned<Vec2f>();
		}
	}

	if (aJson.contains("Ints"))
	{
		for (const auto& intName : aJson["Ints"])
		{
			myInts[intName] = MakeOwned<int>();
		}
	}

	if (aJson.contains("Layers"))
	{
		for (const auto& layerJson : aJson["Layers"])
		{
			const std::string layerName = layerJson["Name"];

			auto layer = MakeOwned<AnimationMachineLayer>(layerName);

			if (layer->InitFromJson(*this, layerJson))
			{
				myLayers.push_back(std::move(layer));
			}
		}
	}

	if (aJson.contains("Model"))
	{
		myModel = GResourceManager->CreateRef<ModelResource>(aJson["Model"]);
		myModel->RequestLoading();
	}

	if (aJson.contains("BindPose"))
	{
		myBindPose.InitFromJson(aJson["BindPose"]);
	}

	myIsInitialized = true;

	return true;
}

nlohmann::json Engine::AnimationMachine::ToJson() const
{
	nlohmann::json j;

	nlohmann::json layersArray = nlohmann::json::array();

	for (const auto& layer : myLayers)
	{
		layersArray.push_back(layer->ToJson());
	}

	j["Layers"] = layersArray;

	nlohmann::json triggersJson = nlohmann::json::array();

	for (const auto& [name, _] : myTriggers)
	{
		triggersJson.push_back(name);
	}

	j["Triggers"] = triggersJson;

	{
		nlohmann::json conditionsJson = nlohmann::json::array();

		for (const auto& [name, _] : myBools)
		{
			conditionsJson.push_back(name);
		}

		j["Conditions"] = conditionsJson;
	}

	{
		nlohmann::json floatsJson = nlohmann::json::array();

		for (const auto& [name, _] : myFloats)
		{
			floatsJson.push_back(name);
		}

		j["Floats"] = floatsJson;
	}

	{
		nlohmann::json vec2sJson = nlohmann::json::array();

		for (const auto& [name, _] : myVec2s)
			for (const auto& [name, _] : myVec2s)
			{
				vec2sJson.push_back(name);
			}

		j["Vec2s"] = vec2sJson;
	}

	{
		nlohmann::json intsJson = nlohmann::json::array();

		for (const auto& [name, _] : myInts)
		{
			intsJson.push_back(name);
		}

		j["Ints"] = intsJson;
	}

	nlohmann::json animationClipArray = nlohmann::json::array();

	for (const auto& clip : myAnimationClips)
	{
		animationClipArray.push_back(clip->GetPath());
	}

	j["AnimationClips"] = animationClipArray;

	if (myModel && myModel->IsValid())
	{
		j["Model"] = myModel->GetPath();
	}

	j["BindPose"] = myBindPose.ToJson();

	return j;
}

Engine::AnimationTransformResult Engine::AnimationMachine::Update(const float aDeltaTime, AnimationController& aController)
{
	ZoneScopedN("AnimationMachine::Update");

	if (myBindPose.myEntries.empty())
	{
		LOG_WARNING(LogType::Animation) << "Bind pose is null";
		return {};
	}

	// We started at bindpose so the first layer can blend towards this?
	AnimationTransformResult totalResult = myBindPose;

	for (size_t i = 0; i < myLayers.size(); ++i)
	{
		const auto& layer = myLayers[i];

		const AnimationTransformResult layerResult = layer->Update(aDeltaTime, aController);

		// if we are empty, means there was a state that had no playable which is valid
		// so just skip it
		if (layerResult.myEntries.empty())
		{
			continue;
		}

		// Skeleton mismatch
		// assert(layerResult.myEntries.size() == totalResult.myEntries.size());
		if (layerResult.myEntries.size() != totalResult.myEntries.size())
		{
			LOG_ERROR(LogType::Animation) << "Skeleton mismatch. Layer: " << layer->GetName();
			continue;
		}

		switch (layer->GetType())
		{
		case AnimationMachineLayer::Type::Override:
		{
			ZoneNamedN(zone1, "AnimationMachine::Override", true);
			if (layer->GetBlendMask().IsEmpty())
			{
				totalResult = BlendHelper::Blend(totalResult, layerResult, layer->GetWeight());
			}
			else
			{
				totalResult = BlendHelper::BlendMasked(totalResult, layerResult, layer->GetWeight(), layer->GetBlendMask());
			}
		} break;

		case AnimationMachineLayer::Type::Additive:
		{
			ZoneNamedN(zone1, "AnimationMachine::Additive", true);
			// NEW:
			// Simply get the result from this layer
			// and simply subtract it from a base pose
			// and add onto prev layer results

			// Base Pose = SK-modellen

			// PreviousLayer + (CurrentLayer - BasePose)

			if (layer->GetBlendMask().IsEmpty())
			{
				totalResult = BlendHelper::BlendAdditive(
					totalResult,
					layerResult,
					myBindPose,
					layer->GetWeight());
			}
			else
			{
				totalResult = BlendHelper::BlendAdditiveMasked(
					totalResult,
					layerResult,
					myBindPose,
					layer->GetWeight(),
					layer->GetBlendMask());
			}

		} break;

		default:
			assert(false);
			break;
		}
	}

	return totalResult;
}

const AnimationClipRef& Engine::AnimationMachine::GetFirstAnimationClip() const
{
	return myFirstAnimationClip;
}

void Engine::AnimationMachine::Trigger(const std::string& aTriggerName)
{
	const auto findResult = myTriggers.find(aTriggerName);

	if (findResult != myTriggers.end())
	{
		*findResult->second = true;
	}
	else
	{
		LOG_ERROR(LogType::Animation) << "Trigger " << aTriggerName << " does not exist";
	}
}

void Engine::AnimationMachine::RemoveTrigger(const std::string& aTriggerName)
{
	for (auto& layer : myLayers)
	{
		layer->RemoveTriggerUsage(aTriggerName);
	}

	myTriggers.erase(aTriggerName);
}

bool Engine::AnimationMachine::HasTrigger(const std::string& aTriggerName) const
{
	const auto findResult = myTriggers.find(aTriggerName);

	if (findResult != myTriggers.end())
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool Engine::AnimationMachine::HasBool(const std::string& aConditionName) const
{
	const auto findResult = myBools.find(aConditionName);

	if (findResult != myBools.end())
	{
		return true;
	}
	else
	{
		return false;
	}
}

void Engine::AnimationMachine::SetBool(const std::string& aBoolName, const bool aValue)
{
	const auto findResult = myBools.find(aBoolName);

	if (findResult != myBools.end())
	{
		*findResult->second = aValue;
	}
	else
	{
		LOG_ERROR(LogType::Animation) << "Bool " << aBoolName << " does not exist";
	}
}

bool Engine::AnimationMachine::GetBool(const std::string& aBoolName) const
{
	const auto findResult = myBools.find(aBoolName);

	if (findResult != myBools.end())
	{
		return *findResult->second;
	}
	else
	{
		LOG_ERROR(LogType::Animation) << "Bool " << aBoolName << " does not exist";
		return false;
	}
}

void Engine::AnimationMachine::RemoveBool(const std::string& aBoolName)
{
	for (auto& layer : myLayers)
	{
		layer->RemoveBoolUsage(aBoolName);
	}

	myBools.erase(aBoolName);
}

void Engine::AnimationMachine::SetFloat(const std::string& aFloatName, const float aValue)
{
	const auto findResult = myFloats.find(aFloatName);

	if (findResult != myFloats.end())
	{
		*findResult->second = aValue;
	}
	else
	{
		LOG_ERROR(LogType::Animation) << "Float " << aFloatName << " does not exist";
	}
}

float Engine::AnimationMachine::GetFloat(const std::string& aFloatName) const
{
	const auto findResult = myFloats.find(aFloatName);

	if (findResult != myFloats.end())
	{
		return *findResult->second;
	}
	else
	{
		LOG_ERROR(LogType::Animation) << "Float " << aFloatName << " does not exist";
		return 0.f;
	}
}

void Engine::AnimationMachine::RemoveFloat(const std::string& aFloatName)
{
	for (auto& layer : myLayers)
	{
		layer->RemoveFloatUsage(aFloatName);
	}

	myFloats.erase(aFloatName);
}

void Engine::AnimationMachine::SetInt(const std::string& aIntName, const int aValue)
{
	const auto findResult = myInts.find(aIntName);

	if (findResult != myInts.end())
	{
		*findResult->second = aValue;
	}
	else
	{
		LOG_ERROR(LogType::Animation) << "Int " << aIntName << " does not exist";
	}
}

int Engine::AnimationMachine::GetInt(const std::string& aIntName) const
{
	const auto findResult = myInts.find(aIntName);

	if (findResult != myInts.end())
	{
		return *findResult->second;
	}
	else
	{
		LOG_ERROR(LogType::Animation) << "Int " << aIntName << " does not exist";
		static int empty = 0;
		return empty;
	}
}

void Engine::AnimationMachine::RemoveInt(const std::string& aIntName)
{
	for (auto& layer : myLayers)
	{
		layer->RemoveIntUsage(aIntName);
	}

	myInts.erase(aIntName);
}

void Engine::AnimationMachine::SetVec2(const std::string& aVecName, const Vec2f& aValue)
{
	const auto findResult = myVec2s.find(aVecName);

	if (findResult != myVec2s.end())
	{
		*findResult->second = aValue;
	}
	else
	{
		LOG_ERROR(LogType::Animation) << "Vec2 " << aVecName << " does not exist";
	}
}

const Vec2f& Engine::AnimationMachine::GetVec2(const std::string& aVecName) const
{
	const auto findResult = myVec2s.find(aVecName);

	if (findResult != myVec2s.end())
	{
		return *findResult->second;
	}
	else
	{
		LOG_ERROR(LogType::Animation) << "Vec2 " << aVecName << " does not exist";
		static Vec2f empty;
		return empty;
	}
}

void Engine::AnimationMachine::RemoveVec2(const std::string& aVec2Name)
{
	for (auto& layer : myLayers)
	{
		layer->RemoveVec2Usage(aVec2Name);
	}

	myVec2s.erase(aVec2Name);
}

const std::unordered_map<std::string, Owned<bool>>& Engine::AnimationMachine::GetTriggers() const
{
	return myTriggers;
}

const std::unordered_map<std::string, Owned<bool>>& Engine::AnimationMachine::GetBools() const
{
	return myBools;
}

const std::unordered_map<std::string, Owned<float>>& Engine::AnimationMachine::GetFloats() const
{
	return myFloats;
}

const std::unordered_map<std::string, Owned<Vec2f>>& Engine::AnimationMachine::GetVec2s() const
{
	return myVec2s;
}

const std::unordered_map<std::string, Owned<int>>& Engine::AnimationMachine::GetInts() const
{
	return myInts;
}

std::vector<Owned<Engine::AnimationMachineLayer>>& Engine::AnimationMachine::GetLayers()
{
	return myLayers;
}

Engine::AnimationMachineLayer* Engine::AnimationMachine::FindLayer(const std::string& aName)
{
	for (const auto& layer : myLayers)
	{
		if (layer->GetName() == aName)
		{
			return layer.get();
		}
	}

	return nullptr;
}

void Engine::AnimationMachine::RemoveLayer(const AnimationMachineLayer* aLayer)
{
	auto findResult = std::find_if(myLayers.begin(), myLayers.end(),
		[aLayer](const Owned<AnimationMachineLayer>& l)
		{
			return l.get() == aLayer;
		});
	if (findResult != myLayers.end())
	{
		myLayers.erase(findResult);
	}
}

bool Engine::AnimationMachine::IsInitialized() const
{
	return myIsInitialized;
}

void Engine::AnimationMachine::ClearBindPose()
{
	myBindPose.myEntries.clear();
}

void Engine::AnimationMachine::GenerateBindPose(const Model& aModel)
{
	if (!GetFirstAnimationClip() || !GetFirstAnimationClip()->IsValid())
	{
		// LOG_ERROR(LogType::Animation) << "Unable to generate bind pose because missing first animation clip";
		return;
	}

	GenerateBindPose(aModel, GetFirstAnimationClip()->Get(), myBindPose);
}

void Engine::AnimationMachine::VisitPlayables(PlayableVisitor& aVisitor)
{
	for (auto& layer : myLayers)
	{
		for (const auto& [_, state] : layer->myStates)
		{
			if (state->HasPlayable())
			{
				state->GetPlayable().Accept(aVisitor);
			}
		}
	}
}

const Engine::AnimationTransformResult& Engine::AnimationMachine::GetBindPose() const
{
	return myBindPose;
}

void Engine::AnimationMachine::GenerateBindPose(
	const Model& aModel,
	const AnimationClip& aClip,
	AnimationTransformResult& aBindPose)
{
	aBindPose.myEntries.clear();

	const auto& boneNodes = aClip.GetBoneNodes();

	for (int i = 0; i < boneNodes.size(); ++i)
	{
		AnimationTransform localAnimTransform;

		const auto& nodes = aModel.GetNodes();

		auto fr = std::find_if(nodes.begin(), nodes.end(),
			[&](const FbxNode& aN)
			{
				return boneNodes[i].myName == aN.myName;
			});

		if (fr != nodes.end())
		{
			localAnimTransform.myTranslation = fr->myTransform.GetPositionLocal();
			localAnimTransform.myRotation = fr->myTransform.GetRotationLocal();
			localAnimTransform.myScale = fr->myTransform.GetScaleLocal();
			localAnimTransform.myDebugData = boneNodes[i].myName;
		}

		aBindPose.AddEntry(localAnimTransform);
	}
}
