#pragma once

#include "Component.h"
#include "Engine/Renderer/Model/ModelInstance.h"

namespace Engine
{
	class MeshInstance;
	class ModelInstance;
	class Material;
	class MeshMaterial;

	enum class RendererType;
}

namespace Engine
{
	class ModelComponent : public Component
	{
		COMPONENT(ModelComponent, "ModelComponent");

	public:
		ModelComponent() = default;
		ModelComponent(GameObject* aGameObject);

		ModelComponent(
			GameObject* aGameObject,
			const std::string& aModelPath,
			const bool aByFileName = false);

		virtual ~ModelComponent();

		void Start() override;

		void MoveToUIScene();

		void SetModel(
			const std::string& aModelPath,
			const bool aByFileName = false);

		// Sets material for the model, and not just this model instance
		// meaning all other model instances using this Model, is being set as well
		void SetMaterial(const std::string& aMaterialName);
		void SetMaterial(const MaterialRef& aMaterial);

		void SetMaterialForMesh(const std::string& aMaterialName, const int aMeshIndex);
		void SetMaterialForMesh(const MaterialRef& aMaterial, const int aMeshIndex);

		MeshMaterialInstance& GetMeshMaterialInstanceByIndex(const int aMeshIndex);
		std::string GetModelPath() const;
		ModelInstance& GetModelInstance();
		int GetMeshCount() const;

		void Execute(eEngineOrder aOrder) override;
		void Render() override;

		// All meshes
		void SetCastShadowsAllMeshes(const bool aCastShadows);
		void SetMeshCastShadows(const int aMeshIndex, const bool aCastShadows);
		bool IsMeshCastingShadows(const int aMeshIndex);

		void Reflect(Reflector& aReflector) override;

		bool IsCullingDisabled() const;

		const ModelRef& GetModel() const;
		C::Sphere<float> CalculateWorldBoundingSphere() const;
		std::vector<MeshInstance>& GetMeshInstances();
		const Mat4f& GetTransformMatrix() const;
		bool PassedRenderCheck() const;
		void SetPassedRenderCheck(const bool aPassed);

	protected:
		bool myIsUI = false;

		// for debugging
		std::string myModelPath;

		ModelInstance myModelInstance;

		bool myIsCullingDisabled = false;
	};
}