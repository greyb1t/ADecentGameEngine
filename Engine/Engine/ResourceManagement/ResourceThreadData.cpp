#include "pch.h"
#include "ResourceThreadData.h"
#include "Engine/Renderer/ModelLoader/FbxLoader.h"
#include "Engine/Renderer/Shaders/ShaderLoader.h"
#include "Engine/Renderer/Texture/TextureLoader.h"
#include "Engine/Renderer/Animation/AnimationClipLoader.h"

Engine::ResourceThreadData::ResourceThreadData()
{
}

Engine::ResourceThreadData::~ResourceThreadData()
{
}

Engine::ResourceThreadData::ResourceThreadData(ResourceThreadData&&) = default;

bool Engine::ResourceThreadData::Init(ID3D11Device * aDevice, ID3D11DeviceContext & aContext, ResourceManager & aResourceManagerHandler)
{
	myFbxLoader = MakeOwned<Engine::FbxLoader>(aDevice, aResourceManagerHandler);

	if (!myFbxLoader->Init())
	{
		return false;
	}

	myShaderLoader = MakeOwned<Engine::ShaderLoader>();

	if (!myShaderLoader->Init(aDevice, "Assets\\Shaders\\"))
	{
		return false;
	}

	myTextureLoader = MakeOwned<Engine::TextureLoader>();

	if (!myTextureLoader->Init(aDevice, aContext))
	{
		return false;
	}

	myAnimationClipLoader = MakeOwned<Engine::AnimationClipLoader>();

	if (!myAnimationClipLoader->Init())
	{
		return false;
	}

	return true;
}

Engine::FbxLoader& Engine::ResourceThreadData::GetFbxLoader()
{
	return *myFbxLoader;
}

Engine::ShaderLoader& Engine::ResourceThreadData::GetShaderLoader()
{
	return *myShaderLoader;
}

Engine::TextureLoader& Engine::ResourceThreadData::GetTextureLoader()
{
	return *myTextureLoader;
}

Engine::AnimationClipLoader& Engine::ResourceThreadData::GetAnimationClipLoader()
{
	return *myAnimationClipLoader;
}
