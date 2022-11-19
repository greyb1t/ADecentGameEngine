#pragma once

#include "Engine/ResourceManagement/ResourceRef.h"
#include "../RTransform.h"
#include "../RendererType.h"
// #include "Engine/Renderer/Material/MaterialInstance.h"
#include "MeshInstance.h"
#include "Engine/Renderer/Shaders/ShaderConstants.h"

namespace Engine
{
	class ModelComponent;
}

namespace Engine
{
	class Model;
	class ModelResource;

	class ModelInstance
	{
	public:
		ModelInstance() = default;
		ModelInstance(const ModelInstance& aOther);
		~ModelInstance();

		void Init(const ModelRef& aModel);

		const Mat4f& GetTransformMatrix() const;
		void SetTransformMatrix(const Mat4f& aTransformMatrix);

		const ModelRef& GetModel() const;
		ModelRef& GetModel();

		void SetFinalAnimTransforms(const std::array<Mat4f, ShaderConstants::MaxBoneCount>& aTransforms);
		const std::array<Mat4f, ShaderConstants::MaxBoneCount>& GetFinalAnimTransforms() const;

		bool HasBones() const;

		C::Sphere<float> CalculateWorldBoundingSphere() const;

		bool PassedRenderCheck() const;
		void SetPassedRenderCheck(const bool aPassed);

		std::vector<MeshInstance>& GetMeshInstances();
		const std::vector<MeshInstance>& GetMeshInstances() const;
		MeshInstance& GetMeshInstance(const int aMeshIndex);

		void SetMaterial(const std::string& aMaterialName);

		void SetMaterial(const MaterialRef& aMaterial);

		void SetMaterialForMesh(const std::string& aMaterialName, const int aMeshIndex);
		void SetMaterialForMesh(const MaterialRef& aMaterial, const int aMeshIndex);

	private:
		friend class ModelComponent;

		ModelRef myModel;

		std::vector<MeshInstance> myMeshInstances;

		Mat4f myTransform;

		// TODO: Consider making a vector to not waste space?
		std::array<Mat4f, ShaderConstants::MaxBoneCount> myFinalAnimTransformations;

		bool myPassedRenderCheck = false;
	};
}