#pragma once

#include "Component.h"
#include "Engine/Renderer/Model/ModelInstance.h"
#include "Engine/Renderer/Material/MeshMaterialInstance.h"

namespace Engine
{
	class ModelInstance;
	class Material;

	enum class RendererType;
}

namespace Engine
{
	class MeshComponent : public Component
	{
	public:
		COMPONENT(MeshComponent, "MeshComponent");

		MeshComponent() = default;
		MeshComponent(GameObject* aGameObject);
		MeshComponent(const MeshComponent&) = default;

		virtual ~MeshComponent();

		void SetModel(
			const std::string& aModelPath,
			const bool aByFileName = false);

		void SetMaterial(const std::string& aMaterialName);
		void SetMaterial(const MaterialRef& aMaterial);

		MeshMaterialInstance& GetMaterialInstance();

		void Execute(eEngineOrder aOrder) override;
		void Render() override;

		void SetCastShadows(const bool aCastShadows);
		bool IsCastingShadows() const;

		void SetMeshIndex(const int aMeshIndex);
		int GetMeshIndex() const;

		void Reflect(Reflector& aReflector) override;

		const ModelRef& GetModel() const;

		C::Sphere<float> CalculateWorldBoundingSphere() const;

		bool IsCullingDisabled() const;

	protected:
		ModelRef myModel;

		MeshMaterialInstance myMaterialInstance;

		bool myCastShadows = true;

		int myMeshIndex = 0;

		bool myIsCullingDisabled = false;

		// bool myPassedRenderCheck = false;
	};
}