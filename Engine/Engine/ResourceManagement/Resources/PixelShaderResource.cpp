#include "pch.h"
#include "PixelShaderResource.h"
#include "Engine\Renderer\Shaders\ShaderLoader.h"
#include "Engine\Renderer\Shaders\PixelShader.h"
#include "Engine/Renderer\GraphicsEngine.h"
#include "Engine\Renderer\Material\MaterialFactory.h"
#include "Engine/ResourceManagement/ResourceObserver.h"
#include "Engine\Editor\FileTypes.h"

Engine::PixelShaderResource::PixelShaderResource(
	ResourceManager& aResourceManager,
	const std::string& aPath,
	const ResourceId& aId)
	: ResourceBase(aResourceManager, aPath, aId)
{
	std::filesystem::path tempPath = aPath;

	if (myResourceManager->GetSettings().UsePrecompiledShaders)
	{
		tempPath.replace_extension(".cso");
	}
	else
	{
		tempPath.replace_extension(FileType::Extension_PixelShader);
	}

	myPath = tempPath.string();

	//std::string path = "Assets\\Shaders\\";
	//std::string extension;

	//if (myResourceManager->GetSettings().UsePrecompiledShaders)
	//{
	//	path += "Compiled\\";
	//	extension = ".cso";
	//}
	//else
	//{
	//	path += "Runtime\\";
	//	extension = FileType::Extension_PixelShader;
	//}

	//myPath = path + myPath + extension;
}

Engine::PixelShaderResource::~PixelShaderResource()
{

}

Engine::PixelShaderResource::PixelShaderResource(const PixelShaderResource& aOther) :
	ResourceBase(*aOther.myResourceManager, aOther.myPath, aOther.myIdentifier)
{
	assert(aOther.myPixelShader == nullptr && "why is this not nullptr? it is "
		"supposed to be because we only ever copy this in ResourceManager "
		"before it has been fully created");
}

bool Engine::PixelShaderResource::Create(ResourceThreadData& aThreadData)
{
	Owned<Engine::PixelShader> pixelShader = nullptr;

	if (myResourceManager->GetSettings().UsePrecompiledShaders)
	{
		pixelShader = aThreadData.GetShaderLoader().LoadPixelShader(myPath);
	}
	else
	{
		pixelShader = aThreadData.GetShaderLoader().CompilePixelShader(
			myPath, 
			{ },
			myIncludedFilePaths);
	}

	if (pixelShader == nullptr)
	{
		return false;
	}

	myPixelShader = std::move(pixelShader);

	return true;
}

Engine::PixelShader& Engine::PixelShaderResource::Get() const
{
	return *myPixelShader;
}

void Engine::PixelShaderResource::Swap(ResourceBase* aResourceBase)
{
	// We know for a fact that aResourceBase input argument is of the same class
	// otherwise something went wrong in the ResourceManager
	auto other = reinterpret_cast<PixelShaderResource*>(aResourceBase);

	myPixelShader.swap(other->myPixelShader);

	myIncludedFilePaths = other->myIncludedFilePaths;
}

void Engine::PixelShaderResource::PostCreation(ResourceThreadData& aResourceThreadData)
{
	// We need to watch the includes in the main shader as well if they change
	// If they do, we recompile the main shader
	for (const auto& includedFilePath : myIncludedFilePaths)
	{
		if (includedFilePath.find("Utils.hlsli") != std::string::npos)
		{
			int t = 0;
		}

		myResourceManager->GetFileWatcher()->WatchFile(includedFilePath,
			[this, &aResourceThreadData](const std::string& aPath)
		{
			if (!Create(aResourceThreadData))
			{
				myLoadResult = ResourceLoadResult::Failed;
				LOG_ERROR(LogType::Renderer) << "Failed to compile " << myPath;
			}
			else
			{
				myLoadResult = ResourceLoadResult::Succeeded;
				LOG_INFO(LogType::Renderer) << "Sucessfully recompiled " << myPath;
			}

			// Reload the materials that use this shader to ensure we re-reflect
			// and re-create all reflected constant buffers
			// myResourceManager->GetGraphicsEngine().GetMaterialFactory().ReloadMaterials();
			for (auto observer : myObservers)
			{
				observer->OnResourceReloaded();
			}
		});
	}

	myResourceManager->GetFileWatcher()->WatchFile(myPath,
		[this, &aResourceThreadData](const std::string& aPath)
	{
		if (!Create(aResourceThreadData))
		{
			myLoadResult = ResourceLoadResult::Failed;
			LOG_ERROR(LogType::Renderer) << "Failed to compile " << myPath;
		}
		else
		{
			myLoadResult = ResourceLoadResult::Succeeded;
			LOG_INFO(LogType::Renderer) << "Sucessfully recompiled " << myPath;
		}

		// Reload the materials that use this shader to ensure we re-reflect
		// and re-create all reflected constant buffers
		// myResourceManager->GetGraphicsEngine().GetMaterialFactory().ReloadMaterials();
		for (auto observer : myObservers)
		{
			observer->OnResourceReloaded();
		}
	});
}

void Engine::PixelShaderResource::RequestMe() const
{
	// Sadly this is required to avoid having the ResourceBase::RequestLoading as a NON-template function
	// Why do we want that as a non-template function? To avoid circular include with templated shit..
	// ABSOLUTE DOG
	myResourceManager->RequestLoading<PixelShaderResource>(myIdentifier);
}

void Engine::PixelShaderResource::LoadMe() const
{
	myResourceManager->Load<PixelShaderResource>(myIdentifier);
}

void Engine::PixelShaderResource::UnloadMe()
{
	myPixelShader = nullptr;
}
