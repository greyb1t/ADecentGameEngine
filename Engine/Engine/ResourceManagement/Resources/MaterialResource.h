#pragma once

#include "Engine/ResourceManagement/ResourceManager.h"
#include "Engine/ResourceManagement/ResourceId.h"
#include "Engine/ResourceManagement/ResourceBase.h"

namespace Engine
{
	class Material;
	class MeshMaterial;
	class DecalMaterial;
	class ParticleMaterial;
}

namespace Engine
{
	class MaterialResource : public ResourceBase
	{
	public:
		static inline ResourceType ourResourceType = ResourceType::MeshMaterial;

	public:
		MaterialResource(
			ResourceManager& aResourceManager,
			const std::string& aPath,
			const ResourceId& aId);

		virtual ~MaterialResource();

		MaterialResource(const MaterialResource& aOther);

		Material* Get() const;
		Material* Get();

		// Will returns nullptr if the material is not of mesh type
		MeshMaterial* GetAsMeshMaterial() const;
		MeshMaterial* GetAsMeshMaterial();

		// Will returns nullptr if the material is not of decal type
		DecalMaterial* GetAsDecalMaterial() const;
		DecalMaterial* GetAsDecalMaterial();

		// Will returns nullptr if the material is not of particle type
		ParticleMaterial* GetAsParticleMaterial() const;
		ParticleMaterial* GetAsParticleMaterial();

	protected:
		friend class ResourceManager;

		bool Create(ResourceThreadData& aThreadData) override;
		void Swap(ResourceBase* aResourceBase);
		void PostCreation(ResourceThreadData& aResourceThreadData) override;
		void RequestMe() const;
		void LoadMe() const;
		void UnloadMe();

	private:
		Owned<Material> myMeshMaterial = nullptr;
	};
}
