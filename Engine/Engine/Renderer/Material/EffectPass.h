#pragma once

#include "Engine\ResourceManagement\ResourceRef.h"
#include "EffectPassDesc.h"
#include "EffectPassState.h"
#include "MaterialFactoryContext.h"
#include "Engine/Renderer/RenderEnums.h"
#include "Engine/ResourceManagement/ResourceObserver.h"
#include "EffectTexture.h"
#include "EffectConstantBuffer.h"
#include "Engine\Renderer\BindFlags.h"
#include "Engine\Renderer\ConstantBuffer.h"
#include "Engine\Renderer\Shaders\ShaderStructs.h"
#include "MaterialType.h"

struct ID3D11ShaderReflection;

namespace Engine
{
	extern std::atomic_int myEffectIDCounter;

	class EffectTexture;
	class RenderManager;
	struct MaterialFactoryContext;
	class EffectPass;
	class ResourceManager;
	class Material;

	class EffectPass : public ResourceObserver
	{
	public:
		EffectPass();
		virtual ~EffectPass();

		EffectPass(const EffectPass&) = delete;

		bool InitFromLoaderEffectPass(
			MaterialFactoryContext& aFactoryContext,
			const EffectPassDesc& aLoaderEffectPass);

		void InitFromJson(
			const nlohmann::json& aJson,
			MaterialFactoryContext& aFactoryContext,
			const MaterialType aMaterialType);
		nlohmann::json ToJson() const;

		const std::string& GetName() const;

		const VertexShaderRef& GetVertexShader() const;
		const PixelShaderRef& GetPixelShader() const;
		const GeometryShaderRef& GetGeometryShader() const;

		EffectPassState BindToPipeline(
			ID3D11DeviceContext* aContext,
			const BindEffectFlags aFlags) const;

		void SetFloat4(const std::string& aVariableName, const C::Vector4f& aValue);

		C::Vector4f* GetFloat4(const std::string& aVariableName);

		void MapConstantBuffersDataToGPU(ID3D11DeviceContext& aContext) const;

		std::unordered_map<std::string, Owned<EffectConstantBuffer>>& GetConstantBuffers();

		const std::vector<EffectTexture>& GetTextures();

		Material* GetMaterial();

		void CreateConstantBuffersFromShader(
			ID3D11ShaderReflection* aReflection,
			ID3D11Device* aDevice,
			const ShaderFlags aShaderFlag);

		void CreateTexturesFromReflection(
			ResourceManager& aResourceManager,
			ID3D11ShaderReflection* aReflection,
			ID3D11Device* aDevice);

		void ParseReflection(
			ResourceManager& aResourceManager,
			ID3D11Device* aDevice,
			ID3D11ShaderReflection& aReflection,
			const ShaderFlags aShaderFlag);

		void Validate();

		RendererType GetRendererType() const;

		void OnResourceReloaded() override;

		bool IsInstanced() const;

		uint16_t GetID() const;

		float GetEmissionIntensity() const;

	private:
		void CreateMaterialConstantBuffer(MaterialFactoryContext& aFactoryContext);

	private:
		friend class InspectableMaterial;

		MaterialFactoryContext myContext;

		EffectPassDesc myDesc;

		VertexShaderRef myVertexShader;
		PixelShaderRef myPixelShader;
		GeometryShaderRef myGeometryShader;

		std::vector<EffectTexture> myTextures;
		std::unordered_map<std::string, Owned<EffectConstantBuffer>> myConstantBuffers;

		mutable ConstantBuffer<MaterialData> myMaterialBuffer;

		DepthStencilState myDepthStencilState = DepthStencilState_NotSet;
		UINT myStencilRef = 0;

		// RendererType myRendererType = RendererType::Deferred;

		bool myIsInstanced = false;

		//float myEmissionIntensity = 1.f;

		// ID is used to generate the draw call sortkey
		uint16_t myID = -1;
	};
}
