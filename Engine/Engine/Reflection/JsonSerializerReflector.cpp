#include "pch.h"
#include "JsonSerializerReflector.h"

#include "Enum.h"
#include "Engine/Animation/AnimationPair.h"
#include "Engine/GameObject/Transform.h"
#include "Engine/ResourceManagement/Resources/AnimationCurveResource.h"
#include "Engine/ResourceManagement/Resources/ModelResource.h"
#include "Engine/ResourceManagement/Resources/TextureResource.h"
#include "Engine/ResourceManagement/Resources/AnimationStateMachineResource.h"
#include "Engine/ResourceManagement/Resources/MaterialResource.h"
#include "Engine/ResourceManagement/Resources/VisualScriptResource.h"
#include "Engine/ResourceManagement/Resources/GameObjectPrefabResource.h"
#include "Engine/GraphManager/VisualGraphPair.h"
#include "Engine/ResourceManagement/Resources/VFXResource.h"

Engine::ReflectorResult Engine::JsonSerializerReflector::ReflectInternal(Reflectable& aValue, const std::string& aName,
	const ReflectionFlags aFlags)
{
	// This is a bit scuffed, however necessary for the recursion
	// to work since it works solely on a member variable
	nlohmann::json j = std::move(myJson);

	myJson = { };

	aValue.Reflect(*this);

	j[aName] = std::move(myJson);

	myJson = j;

	return ReflectorResult_None;
}

Engine::ReflectorResult Engine::JsonSerializerReflector::ReflectInternal(GameObjectRef& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	ReflectorResult result = ReflectorResult_None;

	nlohmann::json j;
	j["RefType"] = aValue.myType;

	switch (aValue.myType)
	{
	case GameObjectRef::Type::PrefabHierarchyReference:
		j["RefPrefabGameObjectID"] = aValue.myPrefabHierarchyReference.myPrefabGameObjectID;
		break;
	case GameObjectRef::Type::SceneHierarchyReference:
		j["RefUUID"] = aValue.mySceneHierarchyReference.myUUID;
		break;
	default:
		break;
	}

	myJson[aName] = j;

	return result;
}

Engine::ReflectorResult Engine::JsonSerializerReflector::ReflectInternal(uint64_t& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	ReflectorResult result = ReflectorResult_None;

	myJson[aName] = aValue;

	return result;
}

Engine::ReflectorResult Engine::JsonSerializerReflector::ReflectInternal(
	int& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	ReflectorResult result = ReflectorResult_None;

	myJson[aName] = aValue;

	return result;
}

Engine::ReflectorResult Engine::JsonSerializerReflector::ReflectInternal(
	float& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	ReflectorResult result = ReflectorResult_None;

	myJson[aName] = aValue;

	return result;
}

Engine::ReflectorResult Engine::JsonSerializerReflector::ReflectInternal(
	Vec2f& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	ReflectorResult result = ReflectorResult_None;

	nlohmann::json j;
	j["x"] = aValue.x;
	j["y"] = aValue.y;

	myJson[aName] = j;

	return result;
}

Engine::ReflectorResult Engine::JsonSerializerReflector::ReflectInternal(
	Vec3f& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	ReflectorResult result = ReflectorResult_None;

	nlohmann::json j;
	j["x"] = aValue.x;
	j["y"] = aValue.y;
	j["z"] = aValue.z;

	myJson[aName] = j;

	return result;
}

Engine::ReflectorResult Engine::JsonSerializerReflector::ReflectInternal(
	Vec4f& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	ReflectorResult result = ReflectorResult_None;

	nlohmann::json j;
	j["x"] = aValue.x;
	j["y"] = aValue.y;
	j["z"] = aValue.z;
	j["w"] = aValue.w;

	myJson[aName] = j;

	return result;
}

Engine::ReflectorResult Engine::JsonSerializerReflector::ReflectInternal(
	bool& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	ReflectorResult result = ReflectorResult_None;

	myJson[aName] = aValue;

	return result;
}

Engine::ReflectorResult Engine::JsonSerializerReflector::ReflectInternal(
	std::string& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	ReflectorResult result = ReflectorResult_None;

	myJson[aName] = aValue;

	return result;
}

Engine::ReflectorResult Engine::JsonSerializerReflector::ReflectInternal(
	ModelRef& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	ReflectorResult result = ReflectorResult_None;

	std::string path;

	if (aValue && aValue->IsValid())
	{
		path = aValue->GetPath();
	}

	myJson[aName] = path;

	return result;
}

Engine::ReflectorResult Engine::JsonSerializerReflector::ReflectInternal(
	MaterialRef& aValue,
	const MaterialType aType,
	const std::string& aName,
	const ReflectionFlags aFlags)
{
	ReflectorResult result = ReflectorResult_None;

	std::string path;

	if (aValue && aValue->IsValid())
	{
		path = aValue->GetPath();
	}

	myJson[aName] = path;

	return result;
}

Engine::ReflectorResult Engine::JsonSerializerReflector::ReflectInternal(
	AnimationPair& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	ReflectorResult result = ReflectorResult_None;

	std::string path;

	if (aValue.myAnimationResource && aValue.myAnimationResource->IsValid())
	{
		path = aValue.myAnimationResource->GetPath();
	}

	myJson[aName] = path;

	return result;
}

Engine::ReflectorResult Engine::JsonSerializerReflector::ReflectInternal(
	TextureRef& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	ReflectorResult result = ReflectorResult_None;

	std::string path;

	if (aValue && aValue->IsValid())
	{
		path = aValue->GetPath();
	}

	myJson[aName] = path;

	return result;
}
Engine::ReflectorResult Engine::JsonSerializerReflector::ReflectInternal(
	VisualGraphPair& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	ReflectorResult result = ReflectorResult_None;

	std::string path;

	if (aValue.myVisualGraphResource && aValue.myVisualGraphResource->IsValid())
	{
		path = aValue.myVisualGraphResource->GetPath();
	}

	myJson[aName] = path;

	return result;
}

Engine::ReflectorResult Engine::JsonSerializerReflector::ReflectInternal(
	AnimationCurveRef& aValue,
	const std::string& aName, const ReflectionFlags aFlags)
{
	ReflectorResult result = ReflectorResult_None;

	std::string path;

	if (aValue && aValue->IsValid())
	{
		path = aValue->GetPath();
	}

	myJson[aName] = path;

	return result;
}

Engine::ReflectorResult Engine::JsonSerializerReflector::ReflectInternal(GameObjectPrefabRef& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	ReflectorResult result = ReflectorResult_None;

	std::string path;

	if (aValue && aValue->IsValid())
	{
		path = aValue->GetPath();
	}

	myJson[aName] = path;

	return result;
}

Engine::ReflectorResult Engine::JsonSerializerReflector::ReflectInternal(VFXRef& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	ReflectorResult result = ReflectorResult_None;

	std::string path;

	if (aValue && aValue->IsValid())
	{
		path = aValue->GetPath();
	}

	myJson[aName] = path;

	return result;
}

Engine::ReflectorResult Engine::JsonSerializerReflector::ReflectInternal(
	Transform& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	ReflectorResult result = ReflectorResult_None;

	const auto& localPos = aValue.GetPositionLocal();
	myJson["LocalPos"]["x"] = localPos.x;
	myJson["LocalPos"]["y"] = localPos.y;
	myJson["LocalPos"]["z"] = localPos.z;

	const auto& localRot = aValue.GetRotationLocal();
	myJson["LocalRotQuat"]["x"] = localRot.myVector.x;
	myJson["LocalRotQuat"]["y"] = localRot.myVector.y;
	myJson["LocalRotQuat"]["z"] = localRot.myVector.z;
	myJson["LocalRotQuat"]["w"] = localRot.myW;

	const auto& localScale = aValue.GetScaleLocal();
	myJson["LocalScale"]["x"] = localScale.x;
	myJson["LocalScale"]["y"] = localScale.y;
	myJson["LocalScale"]["z"] = localScale.z;

	return result;
}

Engine::ReflectorResult Engine::JsonSerializerReflector::ReflectInternal(Enum& aValue, const std::string& aName,
	const ReflectionFlags aFlags)
{
	ReflectorResult result = ReflectorResult_None;

	myJson[aName] = aValue.GetValue();

	return result;
}
