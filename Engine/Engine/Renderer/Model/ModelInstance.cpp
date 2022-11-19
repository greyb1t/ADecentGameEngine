#include "pch.h"
#include "ModelInstance.h"
#include "..\ResourceManagement\Resources\ModelResource.h"
#include "Engine/Engine.h"
#include "..\GraphicsEngine.h"
#include "..\Material\Material.h"
#include "Engine/ResourceManagement/Resources/MaterialResource.h"
#include "Engine\Renderer\Material\MaterialFactory.h"
#include "Engine/ResourceManagement\Resources\ModelResource.h"
#include "Engine\Renderer\Model\Model.h"

namespace Engine
{
	ModelInstance::~ModelInstance()
	{
	}

	ModelInstance::ModelInstance(const ModelInstance& aOther)
		: myModel(aOther.myModel)
		, myMeshInstances(aOther.myMeshInstances)
		, myTransform(aOther.myTransform)
		, myFinalAnimTransformations(aOther.myFinalAnimTransformations)
		, myPassedRenderCheck(aOther.myPassedRenderCheck)
	{
		// Update the parent pointer
		for (auto& meshInstance : myMeshInstances)
		{
			meshInstance.myModelInstance = this;
		}
	}

	void ModelInstance::Init(const ModelRef& aModel)
	{
		myModel = aModel;

		myMeshInstances.clear();

		const auto& meshes = aModel->Get().GetMeshDatas();

		// Create each mesh instance
		for (int i = 0; i < meshes.size(); ++i)
		{
			MeshInstance meshInstance;

			if (meshes[i].myDefaultMaterial)
			{
				meshInstance.Init(this, i, meshes[i].myDefaultMaterial);
			}
			else
			{
				meshInstance.Init(this, i, GetEngine().GetGraphicsEngine().GetMaterialFactory().GetDefaultMeshMaterialDeferred());
			}

			myMeshInstances.push_back(meshInstance);
		}
	}

	const Mat4f& ModelInstance::GetTransformMatrix() const
	{
		return myTransform;
	}

	void ModelInstance::SetTransformMatrix(const Mat4f& aTransformMatrix)
	{
		myTransform = aTransformMatrix;
	}

	const ModelRef& ModelInstance::GetModel() const
	{
		return myModel;
	}

	ModelRef& ModelInstance::GetModel()
	{
		return myModel;
	}

	void ModelInstance::SetFinalAnimTransforms(const std::array<Mat4f, ShaderConstants::MaxBoneCount>& aTransforms)
	{
		myFinalAnimTransformations = aTransforms;
	}

	const std::array<Mat4f, ShaderConstants::MaxBoneCount>& ModelInstance::GetFinalAnimTransforms() const
	{
		return myFinalAnimTransformations;
	}

	bool ModelInstance::HasBones() const
	{
		assert(myModel->IsValid() && "must be valid when calling this");
		return myModel->Get().GetBonesCount() > 0;
	}

	C::Sphere<float> ModelInstance::CalculateWorldBoundingSphere() const
	{
		const auto& boundingSphere = myModel->Get().GetBoundingSphere();

		const auto scale = myTransform.DecomposeScale();

		float biggestAxisScale = 1.f;
		biggestAxisScale = std::max(biggestAxisScale, scale.x);
		biggestAxisScale = std::max(biggestAxisScale, scale.y);
		biggestAxisScale = std::max(biggestAxisScale, scale.z);

		const auto worldSpaceBoundingSphere
			= C::Sphere<float>(myTransform.GetTranslation() + boundingSphere.GetCenter(),
				boundingSphere.GetRadius() * biggestAxisScale);

		return worldSpaceBoundingSphere;
	}

	bool ModelInstance::PassedRenderCheck() const
	{
		return myPassedRenderCheck;
	}

	void ModelInstance::SetPassedRenderCheck(const bool aPassed)
	{
		myPassedRenderCheck = aPassed;
	}

	std::vector<MeshInstance>& ModelInstance::GetMeshInstances()
	{
		return myMeshInstances;
	}

	const std::vector<MeshInstance>& ModelInstance::GetMeshInstances() const
	{
		return myMeshInstances;
	}

	MeshInstance& ModelInstance::GetMeshInstance(const int aMeshIndex)
	{
		return myMeshInstances[aMeshIndex];
	}

	void ModelInstance::SetMaterial(const std::string& aMaterialName)
	{
		/*
			Renderer::MeshMaterial& material = GetEngine().
				GetGraphicsEngine().
				GetMaterialFactory().
				GetMeshMaterial(aMaterialName);

			material.RequestMaterial();
			*/

			//Path path = "Assets\\Materials\\Mesh";
			//path = path.AppendPath(aMaterialName);

			//auto matRef = GetEngine().GetResourceManager().CreateRef<Engine::MeshMaterialResource>(path.ToString());

			//matRef->Load();

		auto matRef
			= GetEngine().GetGraphicsEngine().GetMaterialFactory().GetMeshMaterial(aMaterialName);

		SetMaterial(matRef);
	}

	void ModelInstance::SetMaterial(const MaterialRef& aMaterial)
	{
		for (auto& meshInstance : myMeshInstances)
		{
			meshInstance.SetMaterial(aMaterial);
		}
	}

	void ModelInstance::SetMaterialForMesh(const std::string& aMaterialName, const int aMeshIndex)
	{
		myMeshInstances[aMeshIndex].SetMaterial(aMaterialName);

		//Renderer::MeshMaterial& material = GetEngine().
		//	GetGraphicsEngine().
		//	GetMaterialFactory().
		//	GetMeshMaterial(aMaterialName);

		//material.RequestMaterial();

		///*
		//if (static_cast<Renderer::RendererType>(myRendererType) != material.GetRendererType())
		//{
		//	LOG_WARNING(LogType::Engine) << "Cannot give a mesh a material for another renderer, "
		//		"sets all other mesh materials to default material";

		//	SetMaterial(aMaterialName);

		//	return;
		//}
		//*/

		//// TODO: Move to Mesh
		//myModelData.myMeshesMaterials[aMeshIndex] = &material;

		//myMeshes[aMeshIndex].SetRendererType(material.GetRendererType());

		//// Create new instances of the new material for all modelinstances that uses this model
		//for (const auto& modelInstance : myModelInstances)
		//{
		//	Renderer::MeshMaterialInstance materialInstance;
		//	materialInstance.Init(material);

		//	modelInstance->GetMeshMaterialInstances()[aMeshIndex] = materialInstance;
		//}
	}

	void ModelInstance::SetMaterialForMesh(const MaterialRef& aMaterial,
		const int aMeshIndex)
	{
		myMeshInstances[aMeshIndex].SetMaterial(aMaterial);
	}

	// Renderer::RendererType ModelInstance::GetRendererType() const
	// {
	// 	return myRendererType;
	// }
}