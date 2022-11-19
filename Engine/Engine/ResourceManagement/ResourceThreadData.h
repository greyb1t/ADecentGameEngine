#pragma once

namespace Engine
{
	class FbxLoader;
	class ShaderLoader;
	class TextureLoader;
	class AnimationClipLoader;
}

namespace Engine
{
	class ResourceManager;

	// In this class, each thread we created has their own loaders to avoid 
	// having to synchronize stuff in each Resource's Create() method
	// We simply send this struct with the Create() method to
	// that has local data to that thread ONLY
	class ResourceThreadData
	{
	public:
		ResourceThreadData();
		~ResourceThreadData();

		ResourceThreadData(ResourceThreadData&&);

		bool Init(ID3D11Device* aDevice, ID3D11DeviceContext& aContext, ResourceManager& aResourceManagerHandler);

		FbxLoader& GetFbxLoader();
		ShaderLoader& GetShaderLoader();
		TextureLoader& GetTextureLoader();
		AnimationClipLoader& GetAnimationClipLoader();

	private:
		Owned<FbxLoader> myFbxLoader = nullptr;
		Owned<ShaderLoader> myShaderLoader = nullptr;
		Owned<TextureLoader> myTextureLoader = nullptr;
		Owned<AnimationClipLoader> myAnimationClipLoader = nullptr;
	};
}