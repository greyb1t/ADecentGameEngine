#pragma once

#include "Engine/ResourceManagement/ResourceRef.h"
#include "Engine/Renderer/RendererType.h"
#include "MaterialType.h"

struct ID3D11DeviceContext;

namespace Engine
{
	class MaterialWindow;
	class MaterialResource;
	class InspectableMaterial;
}

namespace Engine
{
	class Effect;
	class ResourceManager;
	class EffectPass;
	struct MaterialFactoryContext;

	class Material
	{
	public:
		Material();
		virtual ~Material();

		virtual void InitFromJson(const nlohmann::json& aJson, MaterialFactoryContext& aFactoryContext, MaterialResource& aMaterialResource, const MaterialType aMaterialType);
		virtual nlohmann::json ToJson() const;

		void RequestMaterial() const;

		// const std::string& GetName() const;

		const std::vector<Owned<EffectPass>>& GetPasses() const;

		void SetFloat4(const std::string& aVariableName, const C::Vector4f& aValue);
		C::Vector4f GetFloat4(const std::string& aVariableName);

		bool IsTransparent() const;

		MaterialType GetType() const;

		virtual void Bind(ID3D11DeviceContext& aContext) {}

		bool IsCastingShadows() const;

	private:
		friend class MaterialWindow;
		friend class InspectableMaterial;
		friend class InspectableMaterial;

		MaterialType myType = MaterialType::Unknown;

		// std::string myName;

		std::vector<Owned<EffectPass>> myPasses;

		// This determines if what uses this material is 
		// rendered back-to-front after all other models are drawn
		bool myIsTransparent = false;
		bool myIsCastingShadows = true;

	};
}