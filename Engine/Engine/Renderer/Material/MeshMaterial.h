#pragma once

#include "Material.h"

namespace Engine
{
	class MaterialWindow;
}

namespace Engine
{
	class MeshMaterial : public Material
	{
	public:
		virtual ~MeshMaterial() = default;

		void InitFromJson(
			const nlohmann::json& aJson,
			MaterialFactoryContext& aFactoryContext,
			MaterialResource& aMaterialResource,
			const MaterialType aMaterialType) override;

		nlohmann::json ToJson() const override;

		//bool Init(
		//	MaterialFactoryContext& aFactoryContext,
		//	const LoaderMaterial& aLoaderMaterial) override;

		// RendererType GetRendererType() const;

		void Bind(ID3D11DeviceContext& aContext) override;

	private:
		friend class MaterialWindow;
		friend class InspectableMaterial;

		TextureRef myAlbedoTexture;
		TextureRef myMaterialTexture;
		TextureRef myNormalTexture;
		TextureRef myEmissiveTexture;
		// RendererType myRendererType = RendererType::LocalForward;
	};
}