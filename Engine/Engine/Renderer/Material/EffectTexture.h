#pragma once

#include "Engine\ResourceManagement\ResourceRef.h"
#include "Engine\Renderer\ShaderFlags.h"

struct ID3D11ShaderReflection;

namespace Engine
{
	class ResourceManager;
}

namespace Engine
{
	struct EffectTextureDesc;

	class EffectTexture
	{
	public:
		EffectTexture();
		~EffectTexture();

		bool Init(
			ResourceManager& aResourceManager,
			const EffectTextureDesc& aLoaderTexture,
			ID3D11Device& aDevice,
			ID3D11ShaderReflection& aReflection);

		nlohmann::json ToJson() const;

		TextureRef& GetTextureResource();
		const TextureRef& GetTextureResource() const;

		int GetShaderSlot() const;

		const std::string& GetNameInShader() const;

		void MakeUsedByShader(const ShaderFlags aShader);

		bool IsUsedByShader(const ShaderFlags aShader) const;

	private:
		friend class InspectableMaterial;

		std::string myNameInShader;
		TextureRef myTexture;
		int myShaderSlot = 0;

		ShaderFlags myShadersUsedIn = ShaderFlags_None;
	};
}