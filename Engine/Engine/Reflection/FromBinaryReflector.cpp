#include "pch.h"
#include "FromBinaryReflector.h"
#include "Enum.h"
#include "Engine/GameObject/Transform.h"
#include "Engine/ResourceManagement/Resources/VFXResource.h"
#include "Engine/ResourceManagement/Resources/GameObjectPrefabResource.h"
#include "Engine/ResourceManagement/Resources/AnimationClipResource.h"
#include "Engine/ResourceManagement/Resources/AnimationCurveResource.h"
#include "Engine/ResourceManagement/Resources/AnimationStateMachineResource.h"
#include "Engine/ResourceManagement/Resources/VisualScriptResource.h"
#include "Engine/ResourceManagement/Resources/TextureResource.h"
#include "Engine/ResourceManagement/Resources/MaterialResource.h"
#include "Engine/ResourceManagement/Resources/ModelResource.h"
#include "Engine/GraphManager/VisualGraphPair.h"
#include "Engine/Animation/AnimationPair.h"

Engine::ReflectorResult Engine::FromBinaryReflector::ReflectInternal(Enum& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	aValue.SetValue(myReader.Read<decltype(aValue.GetValue())>());
	return ReflectorResult_Changed | ReflectorResult_Changing;
}

Engine::ReflectorResult Engine::FromBinaryReflector::ReflectInternal(GameObjectRef& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	aValue.myType = static_cast<GameObjectRef::Type>(myReader.Read<int>());

	switch (aValue.myType)
	{
	case GameObjectRef::Type::PrefabHierarchyReference:
		aValue.myPrefabHierarchyReference.myPrefabGameObjectID = myReader.Read<int>();
		break;
	case GameObjectRef::Type::SceneHierarchyReference:
		aValue.mySceneHierarchyReference.myUUID = myReader.Read<int>();
		break;
	default:
		break;
	}

	return ReflectorResult_Changed | ReflectorResult_Changing;
}

Engine::ReflectorResult Engine::FromBinaryReflector::ReflectInternal(Transform& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	aValue.SetPositionLocal(myReader.Read<Vec3f>());
	aValue.SetScaleLocal(myReader.Read<Vec3f>());
	aValue.SetRotationLocal(myReader.Read<Quatf>());
	return ReflectorResult_Changed | ReflectorResult_Changing;
}

Engine::ReflectorResult Engine::FromBinaryReflector::ReflectInternal(VFXRef& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	const char* path = myReader.ReadString();
	aValue = GResourceManager->CreateRef<VFXResource>(path);

	return ReflectorResult_Changed | ReflectorResult_Changing;
}

Engine::ReflectorResult Engine::FromBinaryReflector::ReflectInternal(GameObjectPrefabRef& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	const char* path = myReader.ReadString();
	aValue = GResourceManager->CreateRef<GameObjectPrefabResource>(path);

	return ReflectorResult_Changed | ReflectorResult_Changing;
}

Engine::ReflectorResult Engine::FromBinaryReflector::ReflectInternal(AnimationCurveRef& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	const char* path = myReader.ReadString();
	aValue = GResourceManager->CreateRef<AnimationCurveResource>(path);

	return ReflectorResult_Changed | ReflectorResult_Changing;
}

Engine::ReflectorResult Engine::FromBinaryReflector::ReflectInternal(VisualGraphPair& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	const char* path = myReader.ReadString();
	aValue.myVisualGraphResource = GResourceManager->CreateRef<VisualScriptResource>(path);

	return ReflectorResult_Changed | ReflectorResult_Changing;
}

Engine::ReflectorResult Engine::FromBinaryReflector::ReflectInternal(TextureRef& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	const char* path = myReader.ReadString();
	aValue = GResourceManager->CreateRef<TextureResource>(path);

	return ReflectorResult_Changed | ReflectorResult_Changing;
}

Engine::ReflectorResult Engine::FromBinaryReflector::ReflectInternal(AnimationPair& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	const char* path = myReader.ReadString();
	aValue.myAnimationResource = GResourceManager->CreateRef<AnimationStateMachineResource>(path);

	return ReflectorResult_Changed | ReflectorResult_Changing;
}

Engine::ReflectorResult Engine::FromBinaryReflector::ReflectInternal(MaterialRef& aValue, const MaterialType aType, const std::string& aName, const ReflectionFlags aFlags)
{
	const char* path = myReader.ReadString();
	aValue = GResourceManager->CreateRef<MaterialResource>(path);

	return ReflectorResult_Changed | ReflectorResult_Changing;
}

Engine::ReflectorResult Engine::FromBinaryReflector::ReflectInternal(ModelRef& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	const char* path = myReader.ReadString();
	aValue = GResourceManager->CreateRef<ModelResource>(path);

	return ReflectorResult_Changed | ReflectorResult_Changing;
}

Engine::ReflectorResult Engine::FromBinaryReflector::ReflectInternal(std::string& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	aValue = myReader.ReadString();

	return ReflectorResult_Changed | ReflectorResult_Changing;
}

Engine::ReflectorResult Engine::FromBinaryReflector::ReflectInternal(bool& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	aValue = myReader.Read<bool>();

	return ReflectorResult_Changed | ReflectorResult_Changing;
}

Engine::ReflectorResult Engine::FromBinaryReflector::ReflectInternal(Vec4f& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	aValue = myReader.Read<Vec4f>();
	return ReflectorResult_Changed | ReflectorResult_Changing;
}

Engine::ReflectorResult Engine::FromBinaryReflector::ReflectInternal(Vec3f& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	aValue = myReader.Read<Vec3f>();
	return ReflectorResult_Changed | ReflectorResult_Changing;
}

Engine::ReflectorResult Engine::FromBinaryReflector::ReflectInternal(Vec2f& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	aValue = myReader.Read<Vec2f>();
	return ReflectorResult_Changed | ReflectorResult_Changing;
}

Engine::ReflectorResult Engine::FromBinaryReflector::ReflectInternal(float& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	aValue = myReader.Read<float>();
	return ReflectorResult_Changed | ReflectorResult_Changing;
}

Engine::ReflectorResult Engine::FromBinaryReflector::ReflectInternal(uint64_t& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	aValue = myReader.Read<uint64_t>();
	return ReflectorResult_Changed | ReflectorResult_Changing;
}

Engine::ReflectorResult Engine::FromBinaryReflector::ReflectInternal(int& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	aValue = myReader.Read<int>();
	return ReflectorResult_Changed | ReflectorResult_Changing;
}
