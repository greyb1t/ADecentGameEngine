#include "pch.h"
#include "ResourceRefReflector.h"

#include "Engine/Animation/AnimationPair.h"
#include "Engine/ResourceManagement/Resources/AnimationCurveResource.h"
#include "Engine/ResourceManagement/Resources/TextureResource.h"
#include "Engine/ResourceManagement/Resources/AnimationStateMachineResource.h"
#include "Engine/ResourceManagement/Resources/ModelResource.h"
#include "Engine/ResourceManagement/Resources/MaterialResource.h"
#include "Engine/ResourceManagement/Resources/MaterialResource.h"
#include "Engine/ResourceManagement/Resources/VisualScriptResource.h"
#include "Engine/ResourceManagement/Resources/GameObjectPrefabResource.h"
#include "Engine/ResourceManagement/Resources/VFXResource.h"
#include "Engine/GraphManager/VisualGraphPair.h"

Engine::ResourceRefReflector::ResourceRefReflector()
{

}

Engine::ReflectorResult Engine::ResourceRefReflector::ReflectInternal(
	ModelRef& aValue,
	const std::string& aName, const ReflectionFlags aFlags)
{
	if (aValue && aValue->IsValid())
	{
		auto r = myResRefs.AddModel(aValue->GetPath());
		AddChildren(r);
	}
	return ReflectorResult_None;
}

Engine::ReflectorResult Engine::ResourceRefReflector::ReflectInternal(
	MaterialRef& aValue,
	const MaterialType aType,
	const std::string& aName, const ReflectionFlags aFlags)
{
	if (aValue && aValue->IsValid())
	{
		auto r = myResRefs.AddMeshMaterial(aValue->GetPath());
		AddChildren(r);
	}
	return ReflectorResult_None;
}

Engine::ReflectorResult Engine::ResourceRefReflector::ReflectInternal(
	AnimationPair& aValue,
	const std::string& aName, const ReflectionFlags aFlags)
{
	if (aValue.myAnimationResource && aValue.myAnimationResource->IsValid())
	{
		auto r = myResRefs.AddAnimation(aValue.myAnimationResource->GetPath());
		AddChildren(r);
	}
	return ReflectorResult_None;
}

Engine::ReflectorResult Engine::ResourceRefReflector::ReflectInternal(
	TextureRef& aValue,
	const std::string& aName, const ReflectionFlags aFlags)
{
	if (aValue && aValue->IsValid())
	{
		auto r = myResRefs.AddTexture(aValue->GetPath());
		AddChildren(r);
	}
	return ReflectorResult_None;
}

Engine::ReflectorResult Engine::ResourceRefReflector::ReflectInternal(
	VisualGraphPair& aValue,
	const std::string& aName, const ReflectionFlags aFlags)
{
	if (aValue.myVisualGraphResource && aValue.myVisualGraphResource->IsValid())
	{
		auto r = myResRefs.AddGraph(aValue.myVisualGraphResource->GetPath());
		AddChildren(r);
	}
	return ReflectorResult_None;
}

Engine::ReflectorResult Engine::ResourceRefReflector::ReflectInternal(
	AnimationCurveRef& aValue,
	const std::string& aName, const ReflectionFlags aFlags)
{
	if (aValue && aValue->IsValid())
	{
		auto r = myResRefs.AddAnimationCurve(aValue->GetPath());
		AddChildren(r);
	}
	return ReflectorResult_None;
}

Engine::ReflectorResult Engine::ResourceRefReflector::ReflectInternal(GameObjectPrefabRef& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	if (aValue && aValue->IsValid())
	{
		auto r = myResRefs.AddGameObjectPrefab(aValue->GetPath());
		AddChildren(r);
	}
	return ReflectorResult_None;
}

Engine::ReflectorResult Engine::ResourceRefReflector::ReflectInternal(VFXRef& aValue, const std::string& aName, const ReflectionFlags aFlags)
{
	if (aValue && aValue->IsValid())
	{
		auto r = myResRefs.AddVFX(aValue->GetPath());
		AddChildren(r);
	}
	return ReflectorResult_None;
}

Engine::ResourceReferences& Engine::ResourceRefReflector::GetResourceRefs()
{
	return myResRefs;
}

void Engine::ResourceRefReflector::AddChildren(const ResourceRef<ResourceBase>& aResource)
{
	const auto children = aResource->GetChildren(true);

	for (const auto& child : children)
	{
		if (child && child->IsValid())
		{
			myResRefs.AddGeneralResource(child);
		}
	}
}

const Engine::ResourceReferences& Engine::ResourceRefReflector::GetResourceRefs() const
{
	return myResRefs;
}
