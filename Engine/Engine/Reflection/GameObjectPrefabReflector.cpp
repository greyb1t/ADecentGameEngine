#include "pch.h"
#include "GameObjectPrefabReflector.h"

#include "Engine/Animation/AnimationPair.h"
#include "Engine/ResourceManagement/Resources/AnimationCurveResource.h"
#include "Engine/ResourceManagement/Resources/TextureResource.h"
#include "Engine/ResourceManagement/Resources/AnimationStateMachineResource.h"
#include "Engine/ResourceManagement/Resources/ModelResource.h"
#include "Engine/ResourceManagement/Resources/MaterialResource.h"
#include "Engine/ResourceManagement/Resources/MaterialResource.h"
#include "Engine/ResourceManagement/Resources/VisualScriptResource.h"
#include "Engine/ResourceManagement/Resources/GameObjectPrefabResource.h"
#include "Engine/GraphManager/VisualGraphPair.h"
#include "FindValueReflector.h"
#include "Engine/GameObject/Transform.h"
#include "Enum.h"

Engine::GameObjectPrefabReflector::GameObjectPrefabReflector(Reflectable& aReflectable, Reflectable* aBeforeChangedReflectable)
	: myTargetReflectable(aReflectable),
	myBeforeChangedReflectable(aBeforeChangedReflectable)
{
}

Engine::ReflectorResult Engine::GameObjectPrefabReflector::ReflectInternal(Reflectable& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	// ? unsure if works
	aValue.Reflect(*this);

	return ReflectorResult_None;
}

Engine::ReflectorResult Engine::GameObjectPrefabReflector::ReflectInternal(GameObjectRef& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	DoIt(aValue, aName);

	return ReflectorResult_None;
}

Engine::ReflectorResult Engine::GameObjectPrefabReflector::ReflectInternal(uint64_t& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	DoIt(aValue, aName);

	return ReflectorResult_None;
}

Engine::ReflectorResult Engine::GameObjectPrefabReflector::ReflectInternal(Enum& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	DoIt(aValue, aName);

	return ReflectorResult_None;
}

Engine::ReflectorResult Engine::GameObjectPrefabReflector::ReflectInternal(Transform& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	//assert(false && "must fix so it does not copy parent and other shit, MUST ONLY COPY SCALE, POS, ROT");
	// then call DoIt(aValue, aName);

	DoIt(aValue, aName);

	// TODO: CAll DoIt()
	//FindValueReflector refl(
	//	[&aValue](void* aPtr) -> ReflectorResult
	//	{
	//		auto t = reinterpret_cast<Transform*>(aPtr);

	//		t->SetPositionLocal(aValue.GetPositionLocal());
	//		t->SetRotationLocal(aValue.GetRotationLocal());
	//		t->SetScaleLocal(aValue.GetScaleLocal());

	//		return ReflectorResult_Changed;
	//	}, aName);

	//// Modify the target value with this value
	//myTargetReflectable.Reflect(refl);

	return ReflectorResult_None;
}

Engine::ReflectorResult Engine::GameObjectPrefabReflector::ReflectInternal(GameObjectPrefabRef& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	DoIt(aValue, aName);

	return ReflectorResult_None;
}

Engine::ReflectorResult Engine::GameObjectPrefabReflector::ReflectInternal(VFXRef& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	DoIt(aValue, aName);

	return ReflectorResult_None;
}

Engine::ReflectorResult Engine::GameObjectPrefabReflector::ReflectInternal(AnimationCurveRef& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	DoIt(aValue, aName);

	return ReflectorResult_None;
}

Engine::ReflectorResult Engine::GameObjectPrefabReflector::ReflectInternal(VisualGraphPair& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	DoIt(aValue, aName);

	return ReflectorResult_None;
}

Engine::ReflectorResult Engine::GameObjectPrefabReflector::ReflectInternal(TextureRef& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	DoIt(aValue, aName);

	return ReflectorResult_None;
}

Engine::ReflectorResult Engine::GameObjectPrefabReflector::ReflectInternal(AnimationPair& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	DoIt(aValue, aName);

	return ReflectorResult_None;
}

Engine::ReflectorResult Engine::GameObjectPrefabReflector::ReflectInternal(MaterialRef& aValue, const MaterialType aType, const std::string& aName, const ReflectionFlags aFlags)
{
	DoIt(aValue, aName);

	return ReflectorResult_None;
}

Engine::ReflectorResult Engine::GameObjectPrefabReflector::ReflectInternal(ModelRef& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	DoIt(aValue, aName);

	return ReflectorResult_None;
}

Engine::ReflectorResult Engine::GameObjectPrefabReflector::ReflectInternal(std::string& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	DoIt(aValue, aName);

	return ReflectorResult_None;
}

Engine::ReflectorResult Engine::GameObjectPrefabReflector::ReflectInternal(bool& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	DoIt(aValue, aName);

	return ReflectorResult_None;
}

Engine::ReflectorResult Engine::GameObjectPrefabReflector::ReflectInternal(Vec4f& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	DoIt(aValue, aName);

	return ReflectorResult_None;
}

Engine::ReflectorResult Engine::GameObjectPrefabReflector::ReflectInternal(Vec3f& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	DoIt(aValue, aName);

	return ReflectorResult_None;
}

Engine::ReflectorResult Engine::GameObjectPrefabReflector::ReflectInternal(Vec2f& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	DoIt(aValue, aName);

	return ReflectorResult_None;
}

Engine::ReflectorResult Engine::GameObjectPrefabReflector::ReflectInternal(float& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	DoIt(aValue, aName);

	return ReflectorResult_None;
}

Engine::ReflectorResult Engine::GameObjectPrefabReflector::ReflectInternal(int& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	DoIt(aValue, aName);

	return ReflectorResult_None;
}

