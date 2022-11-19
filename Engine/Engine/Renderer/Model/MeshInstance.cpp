#include "pch.h"
#include "MeshInstance.h"
#include "Engine/Engine.h"
#include "..\GraphicsEngine.h"
#include "Engine\Renderer\Material\MaterialFactory.h"
#include "Engine/ResourceManagement/ResourceManager.h"
#include "Engine/ResourceManagement/Resources/MaterialResource.h"
#include "Engine/Reflection/Reflector.h"

void Engine::MeshInstance::Init(/*Mesh& aMesh, */ModelInstance* aModelInstance, const int aMeshIndex, const MaterialRef& aMaterialRef)
{
	myModelInstance = aModelInstance;
	myMeshIndex = aMeshIndex;

	// TODO: can optimize by getting default material immediately instead of searching for it everytime
	// Set default material
	SetMaterial(aMaterialRef);
}

void Engine::MeshInstance::SetMaterial(const std::string& aMaterialName)
{
	auto matRef = GetEngine().GetGraphicsEngine().GetMaterialFactory().GetMeshMaterial(aMaterialName);

	SetMaterial(matRef);
}

void Engine::MeshInstance::SetMaterial(const MaterialRef& aMaterialResource)
{
	myMaterialInstance.Init(aMaterialResource);
}

Engine::MeshMaterialInstance& Engine::MeshInstance::GetMaterialInstance()
{
	return myMaterialInstance;
}

const Engine::MeshMaterialInstance& Engine::MeshInstance::GetMaterialInstance() const
{
	return myMaterialInstance;
}

void Engine::MeshInstance::SetCastShadows(const bool aCastShadows)
{
	myCastShadows = aCastShadows;
}

bool Engine::MeshInstance::IsCastingShadows() const
{
	if (myMaterialInstance.IsValid())
	{
		return myCastShadows && myMaterialInstance.GetMaterialRef()->Get()->IsCastingShadows();
	}
	return myCastShadows;
}

void Engine::MeshInstance::Reflect(Reflector& aReflector)
{
	myMaterialInstance.Reflect(aReflector, myMeshIndex);

	aReflector.Reflect(myCastShadows, "Cast Shadows");
}
