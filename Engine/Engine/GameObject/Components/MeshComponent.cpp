#include "pch.h"
#include "MeshComponent.h"
#include "Engine/Renderer\Material\MaterialFactory.h"
#include "..\Engine.h"
#include "Engine/Renderer\GraphicsEngine.h"
#include "Engine/Renderer\Model\ModelFactory.h"
#include "Engine/Renderer\Model\ModelInstance.h"
#include "..\GameObject.h"
#include "Engine/Renderer/Scene/RendererScene.h"
#include "..\DebugManager\DebugDrawer.h"
#include "Engine/Reflection/Reflector.h"
#include "Engine/Renderer/Model/Model.h"
#include "Engine/ResourceManagement\Resources\ModelResource.h"
#include "Engine/ResourceManagement/Resources/MaterialResource.h"

Engine::MeshComponent::MeshComponent(GameObject* aGameObject)
	: Component(aGameObject)
{
}

Engine::MeshComponent::~MeshComponent()
{
}

void Engine::MeshComponent::SetModel(
	const std::string& aModelPath,
	const bool aByFileName)
{
	ModelRef model;

	if (aByFileName)
	{
		std::string* path = GetEngine().GetGraphicsEngine().GetModelFactory().ConvertModelFileNameToPath(aModelPath);

		if (path)
		{
			model = GetEngine().GetResourceManager().CreateRef<ModelResource>(*path);
			assert(model->GetState() == ResourceState::Loaded);
		}
		else
		{
			// placeholder model
			model = GetEngine().GetResourceManager().CreateRef<ModelResource>("");
			assert(model->GetState() == ResourceState::Loaded);
		}
	}
	else
	{
		model = GetEngine().GetResourceManager().CreateRef<ModelResource>(aModelPath);
		assert(model->GetState() == ResourceState::Loaded);
	}

	myModel = model;
}

void Engine::MeshComponent::SetMaterial(const std::string& aMaterialName)
{
	auto matRef = GetEngine().GetGraphicsEngine().GetMaterialFactory().GetMeshMaterial(aMaterialName);
	SetMaterial(matRef);
}

void Engine::MeshComponent::SetMaterial(const MaterialRef& aMaterial)
{
	myMaterialInstance.Init(aMaterial);
}

Engine::MeshMaterialInstance& Engine::MeshComponent::GetMaterialInstance()
{
	return myMaterialInstance;
}

void Engine::MeshComponent::Execute(eEngineOrder aOrder)
{
	int test = 0;
}

void Engine::MeshComponent::Render()
{
	// TODO: Dont do this every frame
	// myModelInstance.SetTransformMatrix(myGameObject->GetTransform().GetMatrix());

	if (myModel && myModel->IsValid() && myMaterialInstance.IsValid())
	{
		myGameObject->GetScene()->GetRendererScene().RenderMeshInstance(*this);
	}



	// if (myModelInstance.GetModel()->IsValid())
	// {
	// 	const auto sphere = myModelInstance.CalculateWorldBoundingSphere();
	// 
	// 	GetEngine().GetDebugDrawer().DrawSphere3D(
	// 		DebugDrawFlags::BoundingSpheres,
	// 		sphere.GetCenter(),
	// 		sphere.GetRadius());
	// }
}

void Engine::MeshComponent::SetCastShadows(const bool aCastShadows)
{
	myCastShadows = aCastShadows;
}

bool Engine::MeshComponent::IsCastingShadows() const
{
	return myCastShadows;
}

void Engine::MeshComponent::SetMeshIndex(const int aMeshIndex)
{
	myMeshIndex = aMeshIndex;
}

int Engine::MeshComponent::GetMeshIndex() const
{
	return myMeshIndex;
}

void Engine::MeshComponent::Reflect(Reflector& aReflector)
{
	Component::Reflect(aReflector);
	aReflector.Reflect(myModel, "Model");
	myMaterialInstance.Reflect(aReflector, 0);
	aReflector.Reflect(myMeshIndex, "MeshIndex");
	aReflector.Reflect(myCastShadows, "Cast Shadows");
	aReflector.Reflect(myIsCullingDisabled, "Disabled Culling");
}

const ModelRef& Engine::MeshComponent::GetModel() const
{
	return myModel;
}

C::Sphere<float> Engine::MeshComponent::CalculateWorldBoundingSphere() const
{
	// TODO: Get the bounding sphere for the mesh only, not the whole model
	const auto& boundingSphere = myModel->Get().GetBoundingSphere();

	const auto scale = myGameObject->GetTransform().GetScale();

	float biggestAxisScale = 1.f;
	biggestAxisScale = std::max(biggestAxisScale, scale.x);
	biggestAxisScale = std::max(biggestAxisScale, scale.y);
	biggestAxisScale = std::max(biggestAxisScale, scale.z);

	const auto worldSpaceBoundingSphere =
		C::Sphere<float>(
			myGameObject->GetTransform().GetPosition() + boundingSphere.GetCenter(),
			boundingSphere.GetRadius() * biggestAxisScale);

	return worldSpaceBoundingSphere;
}

bool Engine::MeshComponent::IsCullingDisabled() const
{
	return myIsCullingDisabled;
}
