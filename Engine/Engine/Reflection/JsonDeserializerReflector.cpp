#include "pch.h"
#include "JsonDeserializerReflector.h"

#include "Enum.h"
#include "Engine/Engine.h"
#include "Engine/Animation/AnimationPair.h"
#include "Engine/GameObject/Transform.h"
#include "Engine/ResourceManagement/ResourceManager.h"

#include "Engine/ResourceManagement/Resources/ModelResource.h"
#include "Engine/ResourceManagement/Resources/TextureResource.h"
#include "Engine/ResourceManagement/Resources/AnimationStateMachineResource.h"
#include "Engine/ResourceManagement/Resources/MaterialResource.h"
#include "Engine/ResourceManagement/Resources/AnimationCurveResource.h"
#include "Engine/ResourceManagement/Resources/VisualScriptResource.h"
#include "Engine/ResourceManagement/Resources/GameObjectPrefabResource.h"
#include "Engine/GraphManager/VisualGraphPair.h"
#include "Engine/ResourceManagement/Resources/VFXResource.h"

Engine::JsonDeserializerReflector::JsonDeserializerReflector(
	const nlohmann::json* aJson,
	const bool aLoadResourcesNow)
	: myJson(aJson),
	myLoadResourcesNow(aLoadResourcesNow)
{
}

Engine::ReflectorResult Engine::JsonDeserializerReflector::ReflectInternal(
	Reflectable& aValue,
	const std::string& aName,
	const ReflectionFlags aFlags)
{
	if (!myJson->contains(aName))
	{
		return ReflectorResult_None;
	}

	// backup
	auto temp = myJson;

	// the new JSON
	auto jj = (*myJson)[aName];
	myJson = &jj;

	// recursively read from the new JSON
	aValue.Reflect(*this);

	// Go back to old JSON
	myJson = temp;

	//JsonDeserializerReflector refl(myJson[aName]);
	//refl.Reflect(aValue, aName);

	return ReflectorResult_Changed | ReflectorResult_Changing;
}

Engine::ReflectorResult Engine::JsonDeserializerReflector::ReflectInternal(GameObjectRef& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	// If something was added after it was already saved, it does
	// not crash when deserializing it
	if (!myJson->contains(aName))
	{
		return ReflectorResult_None;
	}

	aValue.myType = static_cast<GameObjectRef::Type>((*myJson)[aName]["RefType"]);

	switch (aValue.myType)
	{
	case GameObjectRef::Type::PrefabHierarchyReference:
		aValue.myPrefabHierarchyReference.myPrefabGameObjectID = (*myJson)[aName]["RefPrefabGameObjectID"];
		break;
	case GameObjectRef::Type::SceneHierarchyReference:
		aValue.mySceneHierarchyReference.myUUID = (*myJson)[aName]["RefUUID"];
		break;
	default:
		break;
	}

	return ReflectorResult_Changed | ReflectorResult_Changing;
}

Engine::ReflectorResult Engine::JsonDeserializerReflector::ReflectInternal(uint64_t& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	// If something was added after it was already saved, it does
	// not crash when deserializing it
	if (!myJson->contains(aName))
	{
		return ReflectorResult_None;
	}

	aValue = (*myJson)[aName];

	return ReflectorResult_Changed | ReflectorResult_Changing;
}

Engine::ReflectorResult Engine::JsonDeserializerReflector::ReflectInternal(
	int& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	// If something was added after it was already saved, it does
	// not crash when deserializing it
	if (!myJson->contains(aName))
	{
		return ReflectorResult_None;
	}

	aValue = (*myJson)[aName];

	return ReflectorResult_Changed | ReflectorResult_Changing;
}

Engine::ReflectorResult Engine::JsonDeserializerReflector::ReflectInternal(
	float& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	// If something was added after it was already saved, it does
	// not crash when deserializing it
	if (!myJson->contains(aName))
	{
		return ReflectorResult_None;
	}

	aValue = (*myJson)[aName];

	return ReflectorResult_Changed | ReflectorResult_Changing;
}

Engine::ReflectorResult Engine::JsonDeserializerReflector::ReflectInternal(Vec2f& aValue,
	const std::string& aName,
	const ReflectionFlags aFlags)
{
	// If something was added after it was already saved, it does
	// not crash when deserializing it
	if (!myJson->contains(aName))
	{
		return ReflectorResult_None;
	}

	aValue.x = (*myJson)[aName]["x"];
	aValue.y = (*myJson)[aName]["y"];

	return ReflectorResult_Changed | ReflectorResult_Changing;
}

Engine::ReflectorResult Engine::JsonDeserializerReflector::ReflectInternal(
	Vec3f& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	// If something was added after it was already saved, it does
	// not crash when deserializing it
	if (!myJson->contains(aName))
	{
		return ReflectorResult_None;
	}

	aValue.x = (*myJson)[aName]["x"];
	aValue.y = (*myJson)[aName]["y"];
	aValue.z = (*myJson)[aName]["z"];

	return ReflectorResult_Changed | ReflectorResult_Changing;
}

Engine::ReflectorResult Engine::JsonDeserializerReflector::ReflectInternal(
	Vec4f& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	// If something was added after it was already saved, it does
	// not crash when deserializing it
	if (!myJson->contains(aName))
	{
		return ReflectorResult_None;
	}

	aValue.x = (*myJson)[aName]["x"];
	aValue.y = (*myJson)[aName]["y"];
	aValue.z = (*myJson)[aName]["z"];

	if ((*myJson)[aName].contains("w"))
	{
		aValue.w = (*myJson)[aName]["w"];
	}

	return ReflectorResult_Changed | ReflectorResult_Changing;
}

Engine::ReflectorResult Engine::JsonDeserializerReflector::ReflectInternal(
	bool& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	// If something was added after it was already saved, it does
	// not crash when deserializing it
	if (!myJson->contains(aName))
	{
		return ReflectorResult_None;
	}

	aValue = (*myJson)[aName];

	return ReflectorResult_Changed | ReflectorResult_Changing;
}

Engine::ReflectorResult Engine::JsonDeserializerReflector::ReflectInternal(
	std::string& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	// If something was added after it was already saved, it does
	// not crash when deserializing it
	if (!myJson->contains(aName))
	{
		return ReflectorResult_None;
	}

	aValue = (*myJson)[aName];

	return ReflectorResult_Changed | ReflectorResult_Changing;
}

Engine::ReflectorResult Engine::JsonDeserializerReflector::ReflectInternal(
	ModelRef& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	// If something was added after it was already saved, it does
	// not crash when deserializing it
	if (!myJson->contains(aName))
	{
		return ReflectorResult_None;
	}

	const std::string path = (*myJson)[aName];

	if (path.empty())
	{
		return ReflectorResult_None;
	}

	aValue = GetEngine().GetResourceManager().CreateRef<ModelResource>(path);

	// Must be done since when loading a GameObjectPrefab when
	// the resources was not loaded causes an assert
	if (myLoadResourcesNow)
	{
		// NOTE(filip): Ever since the fix me and Jesper did with requestNow instead of loadNow
		// causing the resources to be requested instead of loaded now to fix the race condition issue
		// when dragging a prefab into scene hierarcky, resources within that prefab e.g. Models
		// are not properly loaded because in its Reflect() its required to be loaded because
		// it needs mesh count, but its only requested so its not loaded when it reached Reflect()
		// therefore, this is an edge case causing us to call Load() when it has not been requested prior
		// this does not have a race condition either because it has not been requested
		// ugly, but it does the job
		if (aValue->GetState() != ResourceState::Requested)
		{
			aValue->Load();
		}
		else
		{
			// Me and Jesper:
			// ugly fix to the issue where a race condition crash occured when loaded a resource that was already requested
			// Therefore, we request it here as well, and not load immediately
			aValue->RequestLoading();
		}
		//aValue->Load();
	}

	// Must load children now
	// for (auto& c : aValue->GetChildren())
	// {
	// 	c->Load();
	// }

	return ReflectorResult_Changed | ReflectorResult_Changing;
}

Engine::ReflectorResult Engine::JsonDeserializerReflector::ReflectInternal(
	MaterialRef& aValue,
	const MaterialType aType,
	const std::string& aName,
	const ReflectionFlags aFlags)
{
	// If something was added after it was already saved, it does
	// not crash when deserializing it
	if (!myJson->contains(aName))
	{
		return ReflectorResult_None;
	}

	const std::string path = (*myJson)[aName];

	if (path.empty())
	{
		return ReflectorResult_None;
	}

	aValue = GetEngine().GetResourceManager().CreateRef<MaterialResource>(path);

	// Must be done since when loading a GameObjectPrefab when
	// the resources was not loaded causes an assert
	if (myLoadResourcesNow)
	{
		// NOTE(filip): Ever since the fix me and Jesper did with requestNow instead of loadNow
		// causing the resources to be requested instead of loaded now to fix the race condition issue
		// when dragging a prefab into scene hierarcky, resources within that prefab e.g. Models
		// are not properly loaded because in its Reflect() its required to be loaded because
		// it needs mesh count, but its only requested so its not loaded when it reached Reflect()
		// therefore, this is an edge case causing us to call Load() when it has not been requested prior
		// this does not have a race condition either because it has not been requested
		// ugly, but it does the job
		if (aValue->GetState() != ResourceState::Requested)
		{
			aValue->Load();
		}
		else
		{
			// Me and Jesper:
			// ugly fix to the issue where a race condition crash occured when loaded a resource that was already requested
			// Therefore, we request it here as well, and not load immediately
			aValue->RequestLoading();
		}
		//aValue->Load();
	}

	return ReflectorResult_Changed | ReflectorResult_Changing;
}

Engine::ReflectorResult Engine::JsonDeserializerReflector::ReflectInternal(
	AnimationPair& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	// If something was added after it was already saved, it does
	// not crash when deserializing it
	if (!myJson->contains(aName))
	{
		return ReflectorResult_None;
	}

	const std::string path = (*myJson)[aName];

	if (path.empty())
	{
		return ReflectorResult_None;
	}

	aValue.myAnimationResource = GetEngine().GetResourceManager().CreateRef<AnimationStateMachineResource>(path);

	// Must be done since when loading a GameObjectPrefab when
	// the resources was not loaded causes an assert
	if (myLoadResourcesNow)
	{
		// NOTE(filip): Ever since the fix me and Jesper did with requestNow instead of loadNow
		// causing the resources to be requested instead of loaded now to fix the race condition issue
		// when dragging a prefab into scene hierarcky, resources within that prefab e.g. Models
		// are not properly loaded because in its Reflect() its required to be loaded because
		// it needs mesh count, but its only requested so its not loaded when it reached Reflect()
		// therefore, this is an edge case causing us to call Load() when it has not been requested prior
		// this does not have a race condition either because it has not been requested
		// ugly, but it does the job
		if (aValue.myAnimationResource->GetState() != ResourceState::Requested)
		{
			aValue.myAnimationResource->Load();
		}
		else
		{
			// Me and Jesper:
			// ugly fix to the issue where a race condition crash occured when loaded a resource that was already requested
			// Therefore, we request it here as well, and not load immediately
			aValue.myAnimationResource->RequestLoading();
		}
		//aValue->Load();
	}

	return ReflectorResult_Changed | ReflectorResult_Changing;
}

Engine::ReflectorResult Engine::JsonDeserializerReflector::ReflectInternal(
	TextureRef& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	// If something was added after it was already saved, it does
	// not crash when deserializing it
	if (!myJson->contains(aName))
	{
		return ReflectorResult_None;
	}

	const std::string path = (*myJson)[aName];

	if (path.empty())
	{
		return ReflectorResult_None;
	}

	aValue = GetEngine().GetResourceManager().CreateRef<TextureResource>(path);

	// Must be done since when loading a GameObjectPrefab when
	// the resources was not loaded causes an assert
	if (myLoadResourcesNow)
	{
		// NOTE(filip): Ever since the fix me and Jesper did with requestNow instead of loadNow
		// causing the resources to be requested instead of loaded now to fix the race condition issue
		// when dragging a prefab into scene hierarcky, resources within that prefab e.g. Models
		// are not properly loaded because in its Reflect() its required to be loaded because
		// it needs mesh count, but its only requested so its not loaded when it reached Reflect()
		// therefore, this is an edge case causing us to call Load() when it has not been requested prior
		// this does not have a race condition either because it has not been requested
		// ugly, but it does the job
		if (aValue->GetState() != ResourceState::Requested)
		{
			aValue->Load();
		}
		else
		{
			// Me and Jesper:
			// ugly fix to the issue where a race condition crash occured when loaded a resource that was already requested
			// Therefore, we request it here as well, and not load immediately
			aValue->RequestLoading();
		}
		//aValue->Load();
	}

	return ReflectorResult_Changed | ReflectorResult_Changing;
}
Engine::ReflectorResult Engine::JsonDeserializerReflector::ReflectInternal(
	VisualGraphPair& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	// If something was added after it was already saved, it does
	// not crash when deserializing it
	if (!myJson->contains(aName))
	{
		return ReflectorResult_None;
	}

	const std::string path = (*myJson)[aName];

	if (path.empty())
	{
		return ReflectorResult_None;
	}

	aValue.myVisualGraphResource = GetEngine().GetResourceManager().CreateRef<VisualScriptResource>(path);

	// Must be done since when loading a GameObjectPrefab when
	// the resources was not loaded causes an assert
	if (myLoadResourcesNow)
	{
		// NOTE(filip): Ever since the fix me and Jesper did with requestNow instead of loadNow
		// causing the resources to be requested instead of loaded now to fix the race condition issue
		// when dragging a prefab into scene hierarcky, resources within that prefab e.g. Models
		// are not properly loaded because in its Reflect() its required to be loaded because
		// it needs mesh count, but its only requested so its not loaded when it reached Reflect()
		// therefore, this is an edge case causing us to call Load() when it has not been requested prior
		// this does not have a race condition either because it has not been requested
		// ugly, but it does the job
		if (aValue.myVisualGraphResource->GetState() != ResourceState::Requested)
		{
			aValue.myVisualGraphResource->Load();
		}
		else
		{
			// Me and Jesper:
			// ugly fix to the issue where a race condition crash occured when loaded a resource that was already requested
			// Therefore, we request it here as well, and not load immediately
			aValue.myVisualGraphResource->RequestLoading();
		}
		//aValue->Load();
	}

	return ReflectorResult_Changed | ReflectorResult_Changing;
}

Engine::ReflectorResult Engine::JsonDeserializerReflector::ReflectInternal(
	AnimationCurveRef& aValue,
	const std::string& aName, const ReflectionFlags aFlags)
{
	// If something was added after it was already saved, it does
	// not crash when deserializing it
	if (!myJson->contains(aName))
	{
		return ReflectorResult_None;
	}

	const std::string path = (*myJson)[aName];

	if (path.empty())
	{
		return ReflectorResult_None;
	}

	aValue = GetEngine().GetResourceManager().CreateRef<AnimationCurveResource>(path);

	// Must be done since when loading a GameObjectPrefab when
	// the resources was not loaded causes an assert
	if (myLoadResourcesNow)
	{
		// NOTE(filip): Ever since the fix me and Jesper did with requestNow instead of loadNow
		// causing the resources to be requested instead of loaded now to fix the race condition issue
		// when dragging a prefab into scene hierarcky, resources within that prefab e.g. Models
		// are not properly loaded because in its Reflect() its required to be loaded because
		// it needs mesh count, but its only requested so its not loaded when it reached Reflect()
		// therefore, this is an edge case causing us to call Load() when it has not been requested prior
		// this does not have a race condition either because it has not been requested
		// ugly, but it does the job
		if (aValue->GetState() != ResourceState::Requested)
		{
			aValue->Load();
		}
		else
		{
			// Me and Jesper:
			// ugly fix to the issue where a race condition crash occured when loaded a resource that was already requested
			// Therefore, we request it here as well, and not load immediately
			aValue->RequestLoading();
		}
		//aValue->Load();
	}

	return ReflectorResult_Changed | ReflectorResult_Changing;
}

Engine::ReflectorResult Engine::JsonDeserializerReflector::ReflectInternal(GameObjectPrefabRef& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	// If something was added after it was already saved, it does
	// not crash when deserializing it
	if (!myJson->contains(aName))
	{
		return ReflectorResult_None;
	}

	const std::string path = (*myJson)[aName];

	if (path.empty())
	{
		return ReflectorResult_None;
	}

	aValue = GetEngine().GetResourceManager().CreateRef<GameObjectPrefabResource>(path);

	// Must be done since when loading a GameObjectPrefab when
	// the resources was not loaded causes an assert
	if (myLoadResourcesNow)
	{
		// NOTE(filip): Ever since the fix me and Jesper did with requestNow instead of loadNow
		// causing the resources to be requested instead of loaded now to fix the race condition issue
		// when dragging a prefab into scene hierarcky, resources within that prefab e.g. Models
		// are not properly loaded because in its Reflect() its required to be loaded because
		// it needs mesh count, but its only requested so its not loaded when it reached Reflect()
		// therefore, this is an edge case causing us to call Load() when it has not been requested prior
		// this does not have a race condition either because it has not been requested
		// ugly, but it does the job
		if (aValue->GetState() != ResourceState::Requested)
		{
			aValue->Load();
		}
		else
		{
			// Me and Jesper:
			// ugly fix to the issue where a race condition crash occured when loaded a resource that was already requested
			// Therefore, we request it here as well, and not load immediately
			aValue->RequestLoading();
		}
		//aValue->Load();
	}

	return ReflectorResult_Changed | ReflectorResult_Changing;
}

Engine::ReflectorResult Engine::JsonDeserializerReflector::ReflectInternal(VFXRef& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	// If something was added after it was already saved, it does
	// not crash when deserializing it
	if (!myJson->contains(aName))
	{
		return ReflectorResult_None;
	}

	const std::string path = (*myJson)[aName];

	if (path.empty())
	{
		return ReflectorResult_None;
	}

	aValue = GetEngine().GetResourceManager().CreateRef<VFXResource>(path);

	// Must be done since when loading a GameObjectPrefab when
	// the resources was not loaded causes an assert
	if (myLoadResourcesNow)
	{
		// NOTE(filip): Ever since the fix me and Jesper did with requestNow instead of loadNow
		// causing the resources to be requested instead of loaded now to fix the race condition issue
		// when dragging a prefab into scene hierarcky, resources within that prefab e.g. Models
		// are not properly loaded because in its Reflect() its required to be loaded because
		// it needs mesh count, but its only requested so its not loaded when it reached Reflect()
		// therefore, this is an edge case causing us to call Load() when it has not been requested prior
		// this does not have a race condition either because it has not been requested
		// ugly, but it does the job
		if (aValue->GetState() != ResourceState::Requested)
		{
			aValue->Load();
		}
		else
		{
			// Me and Jesper:
			// ugly fix to the issue where a race condition crash occured when loaded a resource that was already requested
			// Therefore, we request it here as well, and not load immediately
			aValue->RequestLoading();
		}
		//aValue->Load();
	}

	return ReflectorResult_Changed | ReflectorResult_Changing;
}

Engine::ReflectorResult Engine::JsonDeserializerReflector::ReflectInternal(
	Transform& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	float xPos = 0.0f;
	float yPos = 0.0f;
	float zPos = 0.0f;

	if (!(*myJson)["LocalPos"].is_null())
	{
		if (!(*myJson)["LocalPos"]["x"].is_null())
			xPos = (*myJson)["LocalPos"]["x"];

		if (!(*myJson)["LocalPos"]["y"].is_null())
			yPos = (*myJson)["LocalPos"]["y"];

		if (!(*myJson)["LocalPos"]["z"].is_null())
			zPos = (*myJson)["LocalPos"]["z"];
	}

	aValue.SetPositionLocal(Vec3f(xPos, yPos, zPos));

	const auto& localRot = aValue.GetRotationLocal();
	const float xRot = (*myJson)["LocalRotQuat"]["x"];
	const float yRot = (*myJson)["LocalRotQuat"]["y"];
	const float zRot = (*myJson)["LocalRotQuat"]["z"];
	const float wRot = (*myJson)["LocalRotQuat"]["w"];

	aValue.SetRotationLocal(Quatf(wRot, Vec3f(xRot, yRot, zRot)));

	const float xScale = (*myJson)["LocalScale"]["x"];
	const float yScale = (*myJson)["LocalScale"]["y"];
	const float zScale = (*myJson)["LocalScale"]["z"];

	aValue.SetScaleLocal(Vec3f(xScale, yScale, zScale));

	return ReflectorResult_Changed | ReflectorResult_Changing;
}

Engine::ReflectorResult Engine::JsonDeserializerReflector::ReflectInternal(Enum& aValue, const std::string& aName,
	const ReflectionFlags aFlags)
{
	if (!myJson->contains(aName))
	{
		return ReflectorResult_None;
	}

	aValue.SetValue((*myJson)[aName]);

	return ReflectorResult_Changed | ReflectorResult_Changing;
}
