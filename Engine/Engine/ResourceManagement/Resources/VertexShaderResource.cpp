#include "pch.h"
#include "VertexShaderResource.h"
#include "Engine\Renderer\Shaders\ShaderLoader.h"
#include "Engine\Renderer\Shaders\VertexShader.h"
#include "Engine/Renderer\GraphicsEngine.h"
#include "Engine\Renderer\Material\MaterialFactory.h"
#include "Engine/ResourceManagement/ResourceObserver.h"
#include "Engine\Editor\FileTypes.h"

Engine::VertexShaderResource::VertexShaderResource(
	ResourceManager& aResourceManager,
	const std::string& aPath,
	const ResourceId& aId,
	const D3D11_INPUT_ELEMENT_DESC* aElements,
	const int aElementCount,
	const std::vector<Engine::ShaderDefines>& aDefines)
	: ResourceBase(
		aResourceManager,
		aPath,
		std::string(aId)),
	myElements(aElements),
	myElementCount(aElementCount),
	myDefines(aDefines)
{
	std::filesystem::path tempPath = aPath;

	if (myResourceManager->GetSettings().UsePrecompiledShaders)
	{
		tempPath.replace_extension(".cso");
	}
	else
	{
		tempPath.replace_extension(FileType::Extension_VertexShader);
	}

	myPath = tempPath.string();

	/*
	std::string path = "Assets\\Shaders\\";
	std::string extension;

	// std::filesystem::path tempPath = myPath;
	//
	// // Remove the extension because we set it manually below
	// if (tempPath.has_extension())
	// {
	// 	tempPath.replace_extension();
	// 	myPath = tempPath.string();
	// }

	if (myResourceManager->GetSettings().UsePrecompiledShaders)
	{
		path += "Compiled\\";
		extension = ".cso";
	}
	else
	{
		path += "Runtime\\";
		extension = FileType::Extension_VertexShader;
	}

	myPath = path + myPath + extension;
	*/
}

Engine::VertexShaderResource::~VertexShaderResource()
{
}

Engine::VertexShaderResource::VertexShaderResource(
	const VertexShaderResource& aOther) :
	ResourceBase(*aOther.myResourceManager, aOther.myPath, aOther.myIdentifier),
	myElementCount(aOther.myElementCount),
	myElements(aOther.myElements),
	myDefines(aOther.myDefines)
{
	assert(aOther.myVertexShader == nullptr && "why is this not nullptr? it is "
		"supposed to be because we only ever copy this in ResourceManager "
		"before it has been fully created");
}

bool Engine::VertexShaderResource::Create(ResourceThreadData& aThreadData)
{
	Owned<Engine::VertexShader> vertexShader = nullptr;

	if (myResourceManager->GetSettings().UsePrecompiledShaders)
	{
		vertexShader = aThreadData.GetShaderLoader().LoadVertexShader(
			myPath,
			myElements,
			myElementCount);
	}
	else
	{
		vertexShader = aThreadData.GetShaderLoader().CompileVertexShader(
			myPath,
			myElements,
			myElementCount,
			myDefines,
			myIncludedFilePaths);
	}

	if (vertexShader == nullptr)
	{
		return false;
	}

	myVertexShader = std::move(vertexShader);

	/*
		having a unique pointer in this class only works in GetResourceNow()
		otherwise, it will destroy the unique ptr when it is destroyed
		it saved only the raw pointer, that gets destroyed into myResource
	*/

	return true;
}

Engine::VertexShader& Engine::VertexShaderResource::Get() const
{
	return *myVertexShader;
}

void Engine::VertexShaderResource::Swap(ResourceBase* aResourceBase)
{
	// We know for a fact that aResourceBase input argument is of the same class
	// otherwise something went wrong in the ResourceManager
	auto other = reinterpret_cast<VertexShaderResource*>(aResourceBase);

	myVertexShader.swap(other->myVertexShader);

	myIncludedFilePaths = other->myIncludedFilePaths;
}

void Engine::VertexShaderResource::PostCreation(ResourceThreadData& aResourceThreadData)
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

void Engine::VertexShaderResource::RequestMe() const
{
	// Sadly this is required to avoid having the ResourceBase::RequestLoading as a NON-template function
	// Why do we want that as a non-template function? To avoid circular include with templated shit..
	// ABSOLUTE DOG
	myResourceManager->RequestLoading<VertexShaderResource>(myIdentifier);
}

void Engine::VertexShaderResource::LoadMe() const
{
	myResourceManager->Load<VertexShaderResource>(myIdentifier);
}

void Engine::VertexShaderResource::UnloadMe()
{
	myVertexShader = nullptr;
}
