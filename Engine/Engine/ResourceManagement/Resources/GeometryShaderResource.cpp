#include "pch.h"
#include "GeometryShaderResource.h"
#include "Engine\Renderer\Shaders\ShaderLoader.h"
#include "Engine\Renderer\Shaders\VertexShader.h"
#include "Engine\Renderer\Shaders\GeometryShader.h"
#include "Engine/Renderer/GraphicsEngine.h"
#include "Engine\Renderer\Material\MaterialFactory.h"
#include "Engine/ResourceManagement/ResourceObserver.h"
#include "Engine\Editor\FileTypes.h"

Engine::GeometryShaderResource::GeometryShaderResource(
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
		tempPath.replace_extension(FileType::Extension_GeometryShader);
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
	//	extension = FileType::Extension_GeometryShader;
	//}

	//myPath = path + myPath + extension;
}

Engine::GeometryShaderResource::~GeometryShaderResource()
{
}

Engine::GeometryShaderResource::GeometryShaderResource(const GeometryShaderResource& aOther)
	: ResourceBase(*aOther.myResourceManager, aOther.myPath, aOther.myIdentifier)
{
	assert(aOther.myGeometryShader == nullptr && "why is this not nullptr? it is "
		"supposed to be because we only ever copy this in ResourceManager "
		"before it has been fully created");
}

bool Engine::GeometryShaderResource::Create(ResourceThreadData& aThreadData)
{
	Owned<Engine::GeometryShader> geometryShader = nullptr;

	if (myResourceManager->GetSettings().UsePrecompiledShaders)
	{
		geometryShader = aThreadData.GetShaderLoader().LoadGeometryShader(myPath);
	}
	else
	{
		geometryShader = aThreadData.GetShaderLoader().CompileGeometryShader(
			myPath,
			{ },
			myIncludedFilePaths);
	}

	if (geometryShader == nullptr)
	{
		return false;
	}

	myGeometryShader = std::move(geometryShader);

	return true;
}

Engine::GeometryShader& Engine::GeometryShaderResource::Get() const
{
	return *myGeometryShader;
}

void Engine::GeometryShaderResource::Swap(ResourceBase* aResourceBase)
{
	// We know for a fact that aResourceBase input argument is of the same class
	// otherwise something went wrong in the ResourceManager
	auto other = reinterpret_cast<GeometryShaderResource*>(aResourceBase);

	myGeometryShader.swap(other->myGeometryShader);

	myIncludedFilePaths = other->myIncludedFilePaths;
}

void Engine::GeometryShaderResource::PostCreation(ResourceThreadData& aResourceThreadData)
{
	// We need to watch the includes in the main shader as well if they change
	// If they do, we recompile the main shader
	for (const auto& includedFilePath : myIncludedFilePaths)
	{
		myResourceManager->GetFileWatcher()->WatchFile(includedFilePath,
			[this, &aResourceThreadData](const std::string& aPath)
		{
			if (!Create(aResourceThreadData))
			{
				// Set the load result manually to make the model disappear when it failed
				// to give some feedback to the shader programmer
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
			//myResourceManager->GetGraphicsEngine().GetMaterialFactory().ReloadMaterials();
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
			LOG_ERROR(LogType::Renderer) << "Failed to compile " << myPath;
		}
		else
		{
			LOG_INFO(LogType::Renderer) << "Sucessfully recompiled " << myPath;
		}

		// Reload the materials that use this shader to ensure we re-reflect
		// and re-create all reflected constant buffers
		//myResourceManager->GetGraphicsEngine().GetMaterialFactory().ReloadMaterials();
		for (auto observer : myObservers)
		{
			observer->OnResourceReloaded();
		}
	});
}

void Engine::GeometryShaderResource::RequestMe() const
{
	// Sadly this is required to avoid having the ResourceBase::RequestLoading as a NON-template function
	// Why do we want that as a non-template function? To avoid circular include with templated shit..
	// ABSOLUTE DOG
	myResourceManager->RequestLoading<GeometryShaderResource>(myIdentifier);
}

void Engine::GeometryShaderResource::LoadMe() const
{
	myResourceManager->Load<GeometryShaderResource>(myIdentifier);
}

void Engine::GeometryShaderResource::UnloadMe()
{
	myGeometryShader = nullptr;
}
