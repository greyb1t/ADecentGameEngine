#pragma once

#include "Material.h"
#include "Engine/ResourceManagement/ResourceRef.h"

namespace Engine
{
	class MaterialWindow;
}

namespace Engine
{
	class DecalMaterial : public Material
	{
	public:
		virtual ~DecalMaterial() = default;

		void InitFromJson(
			const nlohmann::json& aJson,
			MaterialFactoryContext& aFactoryContext,
			MaterialResource& aMaterialResource,
			const MaterialType aMaterialType) override;

		nlohmann::json ToJson() const override;

		const TextureRef& GetAlbedoTexture() const;
		const TextureRef& GetMaterialTexture() const;
		const TextureRef& GetNormalTexture() const;
		const TextureRef& GetEmissiveTexture() const;

	private:
		friend class MaterialWindow;
		friend class InspectableMaterial;

		TextureRef myAlbedoTexture;
		TextureRef myMaterialTexture;
		TextureRef myNormalTexture;
		TextureRef myEmissiveTexture;
	};
}