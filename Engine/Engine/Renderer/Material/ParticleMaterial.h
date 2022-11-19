#pragma once

#include "Material.h"
#include "Engine/ResourceManagement/ResourceRef.h"

namespace Engine
{
	class MaterialWindow;
}

namespace Engine
{
	class ParticleMaterial : public Material
	{
	public:
		virtual ~ParticleMaterial() = default;

		void InitFromJson(
			const nlohmann::json& aJson,
			MaterialFactoryContext& aFactoryContext,
			MaterialResource& aMaterialResource, 
			const MaterialType aMaterialType) override;

	private:
		friend class MaterialWindow;
		friend class InspectableMaterial;
	};
}