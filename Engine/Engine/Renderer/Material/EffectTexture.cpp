#include "pch.h"
#include "EffectTexture.h"
#include "Engine\ResourceManagement\Resources\TextureResource.h"
#include "EffectPassDesc.h"

Engine::EffectTexture::EffectTexture()
{

}

Engine::EffectTexture::~EffectTexture()
{

}

bool Engine::EffectTexture::Init(
	Engine::ResourceManager& aResourceManager,
	const EffectTextureDesc& aLoaderTexture,
	ID3D11Device& aDevice,
	ID3D11ShaderReflection& aReflection)
{
	/*
		how to handle multiple shaders?
		only one instances of a constant buffer needed
		so send in all shader and reflect on them
		but only created constant buffer if not already created?
	*/

	D3D11_SHADER_INPUT_BIND_DESC textureResourceDesc = { };
	HRESULT result = aReflection.GetResourceBindingDescByName(aLoaderTexture.myNameInShader.c_str(), &textureResourceDesc);

	if (FAILED(result))
	{
		// I REMOVE THE COMMENT BECAUSE IT WAS PRINTED FOR BOTH VS AND PS SHADERS, IT IT REDUNANT NOW I THINK
		// LOG_WARNING(LogType::Renderer) << "Unable to find texture \"" << aLoaderTexture.myNameInShader <<
		// 	"\", this can be because you have not used the Texture in HLSL and it being optimized away";

		return false;
	}

	myShaderSlot = static_cast<int>(textureResourceDesc.BindPoint);

	myTexture = aResourceManager.CreateRef<Engine::TextureResource>(
		aLoaderTexture.myTexturePath);

	myTexture->RequestLoading();

	myNameInShader = aLoaderTexture.myNameInShader;

	return true;
}

nlohmann::json Engine::EffectTexture::ToJson() const
{
	nlohmann::json j;

	j["VariableName"] = myNameInShader;

	if (myTexture && myTexture->IsValid())
	{
		j["TexturePath"] = myTexture->GetPath();
	}

	return j;
}

TextureRef& Engine::EffectTexture::GetTextureResource()
{
	return myTexture;
}

const TextureRef& Engine::EffectTexture::GetTextureResource() const
{
	return myTexture;
}

int Engine::EffectTexture::GetShaderSlot() const
{
	return myShaderSlot;
}

void Engine::EffectTexture::MakeUsedByShader(const ShaderFlags aShader)
{
	myShadersUsedIn |= aShader;
}

bool Engine::EffectTexture::IsUsedByShader(const ShaderFlags aShader) const
{
	return (myShadersUsedIn & aShader) != 0;
}

const std::string& Engine::EffectTexture::GetNameInShader() const
{
	return myNameInShader;
}
