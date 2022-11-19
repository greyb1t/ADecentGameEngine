#include "pch.h"
#include "ModelComponent.h"
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
#include "Engine/ResourceManagement/Resources/TextureResource.h"
#include "Engine/ResourceManagement\Resources\ModelResource.h"
#include "Engine/Reflection/ToBinaryReflector.h"
#include "Engine/Reflection/FromBinaryReflector.h"
#include "Engine/CopyComponentUsingReflect.h"

Engine::ModelComponent::ModelComponent(
	GameObject* aGameObject, const std::string& aModelPath, const bool aByFileName)
	: Component(aGameObject)
{
	SetModel(aModelPath, aByFileName);
}

Engine::ModelComponent::ModelComponent(GameObject* aGameObject)
	: Component(aGameObject)
{
}

Engine::ModelComponent::~ModelComponent()
{
}

void Engine::ModelComponent::Start()
{
	// REMOVE: Ugly fix for glass windows
	// REMOVE: Remove in future
	/*
	if (myModelInstance.GetModel() && myModelInstance.GetModel()->IsValid())
	{
		for (size_t i = 0; i < myModelInstance.GetModel()->Get().GetMeshDatas().size(); ++i)
		{
			if (!myModelInstance.GetModel()->Get().GetMeshDatas()[i].myTexture[0])
			{
				continue;
			}

			if (myModelInstance.GetModel()->Get().GetMeshDatas()[i].myTexture[0]->GetPath().find("T_Glas_01") != std::string::npos)
			{
				SetMaterialForMesh("DefaultMeshForward", i);
			}
		}
	}
	*/
}

void Engine::ModelComponent::MoveToUIScene()
{
	myIsUI = true;
}

void Engine::ModelComponent::SetModel(const std::string& aModelPath, const bool aByFileName)
{
	myModelPath = aModelPath;

	ModelRef model;

	// TODO: Everywhere in all components, WE SHOULD NEVER CALL REQUEST LOADING ourselves
	// that is called by the ResourceReferences
	// this is to avoid loading resources at runtime
	// model = GetEngine().GetResourceManager().CreateRef<FbxResource>(aModelPath);

	if (aByFileName)
	{
		std::string* path
			= GetEngine().GetGraphicsEngine().GetModelFactory().ConvertModelFileNameToPath(
				aModelPath);

		if (path)
		{
			model = GetEngine().GetResourceManager().CreateRef<ModelResource>(*path);
			// model->RequestLoading();

			assert(model->GetState() == ResourceState::Loaded);

			//const auto& meshes = model->Get().GetMeshDatas();

			//for (int i = 0; i < meshes.size(); ++i)
			//{
			//	if (meshes[i].myDefaultMaterial)
			//	{
			//		SetMaterialForMesh(meshes[i].myDefaultMaterial, i);
			//	}
			//}
		}
		else
		{
			// placeholder model
			model = GetEngine().GetResourceManager().CreateRef<ModelResource>("");
			// model->RequestLoading();

			// TODO; -händer för att "" saknas i Resource References.
			// därför denna här temporart
			model->Load();

			assert(model->GetState() == ResourceState::Loaded);
		}

		//model = GetEngine().GetGraphicsEngine().GetModelFactory().LoadModelByFileName(
		//	aModelPath,
		//	aModelPath);
	}
	else
	{
		if (aModelPath.empty())
		{
			// placeholder
			model = GetEngine().GetResourceManager().CreateRef<ModelResource>("");
			model->Load();
		}
		else
		{
			model = GetEngine().GetResourceManager().CreateRef<ModelResource>(aModelPath);
		}

		// assert(model->GetState() == ResourceState::Loaded);

		//model = GetEngine().GetGraphicsEngine().GetModelFactory().LoadModel(
		//	aModelPath,
		//	aModelPath);
	}

	if (model->IsValid())
	{
		myModelInstance.Init(model);
	}
	else
	{
		std::string s = aModelPath;

		if (s.empty())
		{
			std::string* path
				= GetEngine().GetGraphicsEngine().GetModelFactory().ConvertModelFileNameToPath(
					aModelPath);

			if (path)
			{
				s = *path;
			}
		}

		LOG_WARNING(LogType::Resource) << "While reflecting model, it had not yet been loaded, this is most likely okay unless you notice a model is missing: " << aModelPath;
	}
}

void Engine::ModelComponent::SetMaterial(const std::string& aMaterialName)
{
	myModelInstance.SetMaterial(aMaterialName);
}

void Engine::ModelComponent::SetMaterial(const MaterialRef& aMaterial)
{
	myModelInstance.SetMaterial(aMaterial);
}

void Engine::ModelComponent::SetMaterialForMesh(
	const std::string& aMaterialName, const int aMeshIndex)
{
	myModelInstance.SetMaterialForMesh(aMaterialName, aMeshIndex);
}

void Engine::ModelComponent::SetMaterialForMesh(const MaterialRef& aMaterial,
	const int aMeshIndex)
{
	myModelInstance.SetMaterialForMesh(aMaterial, aMeshIndex);
}

Engine::MeshMaterialInstance& Engine::ModelComponent::GetMeshMaterialInstanceByIndex(
	const int aMeshIndex)
{
	return myModelInstance.GetMeshInstance(aMeshIndex).GetMaterialInstance();
}

std::string Engine::ModelComponent::GetModelPath() const
{
	return myModelPath;
}
Engine::ModelInstance& Engine::ModelComponent::GetModelInstance()
{
	return myModelInstance;
}

int Engine::ModelComponent::GetMeshCount() const
{
	return myModelInstance.GetMeshInstances().size();
}

void Engine::ModelComponent::Execute(eEngineOrder aOrder)
{
}

void Engine::ModelComponent::Render()
{
	// TODO: Dont do this every frame

	myModelInstance.SetTransformMatrix(myGameObject->GetTransform().GetMatrix());

	if (myModelInstance.GetModel()
		&& myModelInstance.GetModel()->IsValid() /* && myMeshMaterialInstance.IsValid()*/)
	{
		if (myIsUI)
		{
			myGameObject->GetScene()->GetUIRendererScene().RenderModelInstance(*this);
		}
		else
		{
			//for (const auto& meshInstance : myModelInstance.GetMeshInstances())
			//{
			//	Renderer::MeshRenderCommand command;
			//	{
			//		command.myCastShadows = meshInstance.IsCastingShadows();
			//		command.myMeshIndex = meshInstance.GetMeshIndex();
			//		command.myMeshMaterialInstance = meshInstance.GetMaterialInstance();
			//		command.myModel = meshInstance.GetModelInstance()->GetModel();
			//		command.myPassedRenderCheck = &myModelInstance.myPassedRenderCheck;
			//	}
			//
			//	//myGameObject->GetScene()->GetRendererScene().DrawMesh(command);
			//}

			myGameObject->GetScene()->GetRendererScene().RenderModelInstance(*this);
		}

		if (myModelInstance.GetModel()->IsValid())
		{
			const auto sphere = myModelInstance.CalculateWorldBoundingSphere();

			myGameObject->GetScene()->GetRendererScene().GetDebugDrawer().DrawSphere3D(
				DebugDrawFlags::BoundingSpheres, sphere.GetCenter(), sphere.GetRadius());
		}
	}
}

void Engine::ModelComponent::SetCastShadowsAllMeshes(const bool aCastShadows)
{
	for (auto& meshInstance : myModelInstance.GetMeshInstances())
	{
		meshInstance.SetCastShadows(aCastShadows);
	}
}

void Engine::ModelComponent::SetMeshCastShadows(const int aMeshIndex, const bool aCastShadows)
{
	myModelInstance.GetMeshInstance(aMeshIndex).SetCastShadows(aCastShadows);
}

bool Engine::ModelComponent::IsMeshCastingShadows(const int aMeshIndex)
{
	return myModelInstance.GetMeshInstance(aMeshIndex).IsCastingShadows();
}

void Engine::ModelComponent::Reflect(Reflector& aReflector)
{
	Component::Reflect(aReflector);
	//aReflector.Reflect(myIsUI, "Is UI");

	aReflector.Reflect(myIsCullingDisabled, "Disable Culling");

	// TODO: dont do it now
	// wait until did MeshComponent
	// have as much in the MeshComponent class as possible,
	// in other words, do not use a MeshInstance member variable

	// aReflector.Reflect();
	ReflectorResult modelResult = aReflector.Reflect(myModelInstance.GetModel(), "Model");

	if (modelResult & ReflectorResult_Changed)
	{
		if (myModelInstance.GetModel())
		{
			SetModel(myModelInstance.GetModel()->GetPath());
		}
	}

	// NOTE(filip): this make the array SIZE hidden only which is the 
	// intended behaviour
	aReflector.SetNextItemHidden();
	aReflector.Reflect(
		myModelInstance.myMeshInstances,
		"Meshes");

	// Updates instances to the default model format.
	if (aReflector.Button("Update instances"))
	{
		if (myModelPath == "")
			return;

		for (const auto& ob : myGameObject->GetScene()->GetGameObjects())
		{
			if (auto* comp = ob->GetComponent<ModelComponent>())
			{
				if (comp->myModelPath == myModelPath)
				{
					comp->SetModel(myModelPath);
				}
			}
		}
	}

	if (aReflector.Button("click"))
	{
		auto r = GResourceManager->CreateRef<ModelResource>("Assets/Meshes/Environment/Landscapes/VtxColorTest.fbx");
		r->Load();
		SetModel("Assets/Meshes/Environment/Landscapes/VtxColorTest.fbx");
	}
}

bool Engine::ModelComponent::IsCullingDisabled() const
{
	return myIsCullingDisabled;
}

const ModelRef& Engine::ModelComponent::GetModel() const
{
	return myModelInstance.GetModel();
}

C::Sphere<float> Engine::ModelComponent::CalculateWorldBoundingSphere() const
{
	return myModelInstance.CalculateWorldBoundingSphere();
}

std::vector<Engine::MeshInstance>& Engine::ModelComponent::GetMeshInstances()
{
	return myModelInstance.GetMeshInstances();
}

const Mat4f& Engine::ModelComponent::GetTransformMatrix() const
{
	return myModelInstance.GetTransformMatrix();
}

bool Engine::ModelComponent::PassedRenderCheck() const
{
	return myModelInstance.PassedRenderCheck();
}

void Engine::ModelComponent::SetPassedRenderCheck(const bool aPassed)
{
	myModelInstance.SetPassedRenderCheck(aPassed);
}
