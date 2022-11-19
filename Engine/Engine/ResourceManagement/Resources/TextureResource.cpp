#include "pch.h"
#include "TextureResource.h"
#include "Engine\Renderer\Shaders\ShaderLoader.h"
#include "Engine\Renderer\Shaders\PixelShader.h"
#include "Engine\Renderer\Texture\TextureLoader.h"
#include "Engine\Renderer\Texture\Texture2D.h"
#include "Engine\Editor\FileTypes.h"

Engine::TextureResource::TextureResource(
	ResourceManager& aResourceManager,
	const std::string& aPath,
	const ResourceId& aId)
	: ResourceBase(aResourceManager, aPath, aId)
{
	// This is a must to getting NULL texture when trying to
	// draw a texture that we have requested, while it is being retrieved
	// replace it with the placeholder
	myTexture = aResourceManager.GetPlaceholderTexture();
	assert(myTexture && "default texture cannot be null");
}

Engine::TextureResource::TextureResource(
	Shared<Engine::Texture2D> aTexture)
	: myTexture(aTexture)
{
	// Otherwise IsValid() will return false
	myState = ResourceState::Loaded;
	myLoadResult = ResourceLoadResult::Succeeded;
}

Engine::TextureResource::TextureResource(const TextureResource& aOther)
	: ResourceBase(*aOther.myResourceManager, aOther.myPath, aOther.myIdentifier)
{
	assert(aOther.myTexture == myResourceManager->GetPlaceholderTexture() && "not allowed to be valid");
}

Engine::TextureResource::~TextureResource()
{

}

Engine::Texture2D& Engine::TextureResource::Get() const
{
	return *myTexture;
}

bool Engine::TextureResource::Create(ResourceThreadData& aThreadData)
{
	Shared<Texture2D> texture;

	const auto extension = std::filesystem::path(myPath).extension().string();

	if (FileType::ExtensionToFileType(extension) == FileType::FileType::DDS)
	{
		texture = aThreadData.GetTextureLoader().LoadTexture(myPath);
	}
	else if (FileType::ExtensionToFileType(extension) == FileType::FileType::HDR)
	{
		texture = aThreadData.GetTextureLoader().LoadHDR(myPath);
	}

	if (texture == nullptr)
	{
		return false;
	}

	myTexture = texture;

	return true;
}

void Engine::TextureResource::Swap(ResourceBase* aResourceBase)
{
	// We know for a fact that aResourceBase input argument is of the same class
	// otherwise something went wrong in the ResourceManager
	auto other = reinterpret_cast<TextureResource*>(aResourceBase);

	myTexture.swap(other->myTexture);
}

void Engine::TextureResource::PostCreation(ResourceThreadData& aResourceThreadData)
{
	myResourceManager->GetFileWatcher()->WatchFile(myPath,
		[this, &aResourceThreadData](const std::string& aPath)
		{
			// this lambda will be called from render thread (main thread)
			// so its ok to edit stuff here because renderer 
			// won't touch the sprite during this time
			Create(aResourceThreadData);
		});
}

void Engine::TextureResource::RequestMe() const
{
	// Sadly this is required to avoid having the ResourceBase::RequestLoading as a NON-template function
	// Why do we want that as a non-template function? To avoid circular include with templated shit..
	// ABSOLUTE DOG
	myResourceManager->RequestLoading<TextureResource>(myIdentifier);
}

void Engine::TextureResource::LoadMe() const
{
	myResourceManager->Load<TextureResource>(myIdentifier);
}

void Engine::TextureResource::UnloadMe()
{
	// a unloaded texture has a placeholder texture
	myTexture = myResourceManager->GetPlaceholderTexture();
}
