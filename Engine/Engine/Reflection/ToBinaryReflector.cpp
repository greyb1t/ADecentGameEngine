#include "pch.h"
#include "ToBinaryReflector.h"
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

Engine::ReflectorResult Engine::ToBinaryReflector::ReflectInternal(Enum& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	myWriter.Write(aValue.GetValue());
	//return ReflectorResult_None;
	return ReflectorResult_None;
}

Engine::ReflectorResult Engine::ToBinaryReflector::ReflectInternal(GameObjectRef& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	myWriter.Write<int>(static_cast<int>(aValue.myType));

	switch (aValue.myType)
	{
	case GameObjectRef::Type::PrefabHierarchyReference:
		myWriter.Write<int>(aValue.myPrefabHierarchyReference.myPrefabGameObjectID);
		break;
	case GameObjectRef::Type::SceneHierarchyReference:
		myWriter.Write<int>(aValue.mySceneHierarchyReference.myUUID);
		break;
	default:
		break;
	}

	return ReflectorResult_None;
}

Engine::ReflectorResult Engine::ToBinaryReflector::ReflectInternal(Transform& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	myWriter.Write(aValue.GetPositionLocal());
	myWriter.Write(aValue.GetScaleLocal());
	myWriter.Write(aValue.GetRotationLocal());
	return ReflectorResult_None;
}

Engine::ReflectorResult Engine::ToBinaryReflector::ReflectInternal(VFXRef& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	if (aValue && aValue->IsValid())
	{
		myWriter.WriteString(aValue->GetPath());
	}
	else
	{
		// Must write empty to the FromBinaryReflector 
		// can read it becase it does not know if it was valid or not
		myWriter.WriteString("");
	}
	return ReflectorResult_None;
}

Engine::ReflectorResult Engine::ToBinaryReflector::ReflectInternal(GameObjectPrefabRef& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	if (aValue && aValue->IsValid())
	{
		myWriter.WriteString(aValue->GetPath());
	}
	else
	{
		// Must write empty to the FromBinaryReflector 
		// can read it becase it does not know if it was valid or not
		myWriter.WriteString("");
	}
	return ReflectorResult_None;
}

Engine::ReflectorResult Engine::ToBinaryReflector::ReflectInternal(AnimationCurveRef& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	if (aValue && aValue->IsValid())
	{
		myWriter.WriteString(aValue->GetPath());
	}
	else
	{
		// Must write empty to the FromBinaryReflector 
		// can read it becase it does not know if it was valid or not
		myWriter.WriteString("");
	}
	return ReflectorResult_None;
}

Engine::ReflectorResult Engine::ToBinaryReflector::ReflectInternal(VisualGraphPair& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	if (aValue.myVisualGraphResource && aValue.myVisualGraphResource->IsValid())
	{
		myWriter.WriteString(aValue.myVisualGraphResource->GetPath());
	}
	else
	{
		// Must write empty to the FromBinaryReflector 
		// can read it becase it does not know if it was valid or not
		myWriter.WriteString("");
	}
	return ReflectorResult_None;
}

Engine::ReflectorResult Engine::ToBinaryReflector::ReflectInternal(TextureRef& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	if (aValue && aValue->IsValid())
	{
		myWriter.WriteString(aValue->GetPath());
	}
	else
	{
		// Must write empty to the FromBinaryReflector 
		// can read it becase it does not know if it was valid or not
		myWriter.WriteString("");
	}
	return ReflectorResult_None;
}

Engine::ReflectorResult Engine::ToBinaryReflector::ReflectInternal(AnimationPair& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	if (aValue.myAnimationResource && aValue.myAnimationResource->IsValid())
	{
		myWriter.WriteString(aValue.myAnimationResource->GetPath());
	}
	else
	{
		// Must write empty to the FromBinaryReflector 
		// can read it becase it does not know if it was valid or not
		myWriter.WriteString("");
	}
	return ReflectorResult_None;
}

Engine::ReflectorResult Engine::ToBinaryReflector::ReflectInternal(MaterialRef& aValue, const MaterialType aType, const std::string& aName, const ReflectionFlags aFlags)
{
	if (aValue && aValue->IsValid())
	{
		myWriter.WriteString(aValue->GetPath());
	}
	else
	{
		// Must write empty to the FromBinaryReflector 
		// can read it becase it does not know if it was valid or not
		myWriter.WriteString("");
	}
	return ReflectorResult_None;
}

Engine::ReflectorResult Engine::ToBinaryReflector::ReflectInternal(ModelRef& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	if (aValue && aValue->IsValid())
	{
		myWriter.WriteString(aValue->GetPath());
	}
	else
	{
		// Must write empty to the FromBinaryReflector 
		// can read it becase it does not know if it was valid or not
		myWriter.WriteString("");
	}
	return ReflectorResult_None;
}

Engine::ReflectorResult Engine::ToBinaryReflector::ReflectInternal(std::string& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	myWriter.WriteString(aValue);
	return ReflectorResult_None;
}

Engine::ReflectorResult Engine::ToBinaryReflector::ReflectInternal(bool& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	myWriter.Write(aValue);
	return ReflectorResult_None;
}

Engine::ReflectorResult Engine::ToBinaryReflector::ReflectInternal(Vec4f& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	myWriter.Write(aValue);
	return ReflectorResult_None;
}

Engine::ReflectorResult Engine::ToBinaryReflector::ReflectInternal(Vec3f& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	myWriter.Write(aValue);
	return ReflectorResult_None;
}

Engine::ReflectorResult Engine::ToBinaryReflector::ReflectInternal(Vec2f& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	myWriter.Write(aValue);
	return ReflectorResult_None;
}

Engine::ReflectorResult Engine::ToBinaryReflector::ReflectInternal(float& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	myWriter.Write(aValue);
	return ReflectorResult_None;
}

Engine::ReflectorResult Engine::ToBinaryReflector::ReflectInternal(uint64_t& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	myWriter.Write(aValue);
	return ReflectorResult_None;
}

Engine::ReflectorResult Engine::ToBinaryReflector::ReflectInternal(int& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	myWriter.Write(aValue);
	return ReflectorResult_None;
}
