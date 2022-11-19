#include "pch.h"
#include "FolderScene.h"

#include "SceneSerializerJson.h"
#include "Engine/Renderer/Scene/RendererScene.h"
#include "Engine/Engine.h"
#include "Engine/Renderer/GraphicsEngine.h"
#include "Engine/Renderer/Model/ModelFactory.h"
#include "Engine/ResourceReferences.h"
#include "Engine/Renderer/Material/MaterialFactory.h"
#include "Engine/ResourceManagement/Resources/MaterialResource.h"

void Engine::FolderScene::SetFolderPath(const Path& aFolderPath)
{
	myFolderPath = aFolderPath;
}

Engine::FolderScene::~FolderScene()
{
}

Engine::Scene::Result Engine::FolderScene::LoadScene()
{
	SceneSerializerJson jsonSerializer;

	if (!jsonSerializer.Deserialize(*this, GetFolderPath().AppendPath("Scene.scene")))
	{
		LOG_ERROR(LogType::Resource) << "Failed to load scene: " << GetFolderPath();
	}

	return Scene::Result::Succeeded;
}

Engine::Scene::Result Engine::FolderScene::LoadWorld()
{
	Time::PauseTimeScale = 1.f;

	if (!myUnrealProject.Init(*this, myResourceReferences))
	{
		LOG_ERROR(LogType::Engine) << "Failed to init the unreal project";
		return Result::Failed;
	}

	START_TIMER(LoadWorldTimer);

	const auto worldJsonPath = myFolderPath.AppendPath("World.json");

	if (std::filesystem::exists(worldJsonPath.ToWString()))
	{
		if (!myUnrealProject.LoadWorld(worldJsonPath.ToString()))
		{
			LOG_ERROR(LogType::Game) << "Failed to load the world";
			return Result::Failed;
		}
	}

	const float loadWorldMs = END_TIMER_GET_RESULT_MS(LoadWorldTimer);

	LOG_INFO(LogType::Game) << "Load World Ms: " << loadWorldMs;

	return Result::Succeeded;
}

bool Engine::FolderScene::Init()
{
	if (!Scene::Init())
	{
		return false;
	}

	if (!mySceneJson.InitFromJson(myFolderPath.AppendPath("Scene.scene")))
	{
		LOG_ERROR(LogType::Engine) << "Failed to init from: " << myFolderPath;
	}

	//const auto extraJsonPath = myFolderPath.AppendPath("Extra.json");
	//if (!myExtraSceneData.InitFromJson(extraJsonPath))
	//{
	//	LOG_ERROR(LogType::Engine) << "Failed to init from: " << extraJsonPath;
	//}

	//if (!CreateDefaultEnvironmentLight(myExtraSceneData.defaultCubemapPath))
	//{
	//	return false;
	//}

	//GetEnvironmentLight().SetColor(myExtraSceneData.cubemapColor);
	//GetEnvironmentLight().SetColorIntensity(myExtraSceneData.cubemapColorIntensity);
	//GetEnvironmentLight().SetCubemapIntensity(myExtraSceneData.cubemapIntensity);

	// GetRendererScene().SetShadowStrength(myExtraSceneData.shadowStrength);

	// Init fog
	//const auto& fog
	//	= GetEngine().GetFogManager().GetPresets().GetFog//(myExtraSceneData.defaultFogPresetName);
	//GetRendererScene().SetFogData(fog.GetData());

	if (!InitResourceReferences())
	{
		LOG_ERROR(LogType::Engine) << "Failed to load resource refs for: " << myFolderPath;
	}

	auto resRefsWorldPath = myFolderPath.AppendPath("ResourceReferencesWorld.json");
	if (!InitResourceReferencesFromWorld(resRefsWorldPath))
	{
		LOG_ERROR(LogType::Engine) << "Failed to load world resource refs for: " << myFolderPath;
	}

	return true;
}

const Path& Engine::FolderScene::GetFolderPath() const
{
	return myFolderPath;
}

std::string Engine::FolderScene::GetName() const
{
	//return mySceneJson.myName;
	return myFolderPath.GetStem().ToString();
}

const std::string& Engine::FolderScene::GetSceneTypeName() const
{
	return myTypeName;
}

Engine::FolderScene* Engine::FolderScene::Clone() const
{
	return DBG_NEW FolderScene(*this);
}

bool Engine::FolderScene::InitResourceReferencesFromWorld(const Path& aPath)
{
	const Path& path = aPath;

	std::ifstream file(path.ToWString());

	if (!file.is_open())
	{
		return false;
	}

	const bool ignoreComments = true;
	const bool allowExceptions = false;
	nlohmann::json j = nlohmann::json::parse(file, nullptr, allowExceptions, ignoreComments);

	// If failed to parse json file
	if (j.is_discarded())
	{
		LOG_ERROR(LogType::Resource) << "Failed to parse: " << path;
		return false;
	}

	for (const std::string modelFilename : j["ModelFilenames"])
	{
		std::string* modelPath
			= GetEngine().GetGraphicsEngine().GetModelFactory().ConvertModelFileNameToPath(
				modelFilename);

		if (modelPath == nullptr)
		{
			LOG_ERROR(LogType::Resource)
				<< "Unable to convert: " << modelFilename << " to path, means it was not found.";

			continue;
		}

		myResourceReferences.AddModel(*modelPath);
	}

	for (const std::string modelFilename : j["MeshMaterials"])
	{
		auto materialRef = GetEngine().GetGraphicsEngine().GetMaterialFactory().GetMeshMaterial(modelFilename);

		if (!materialRef)
		{
			LOG_ERROR(LogType::Resource)
				<< "Unable to convert: " << modelFilename << " to path, means it was not found.";

			continue;
		}

		myResourceReferences.AddMeshMaterial(materialRef->GetPath());
	}

	return true;
}

bool Engine::FolderScene::InitResourceReferences()
{
	if (!myResourceReferences.AddReferencesFromJson(
		myFolderPath.AppendPath("ResourceReferences.json")))
	{
		LOG_WARNING(LogType::Resource) << "Failed to load resource refs: ResourceReferences.json";
	}

	if (!myResourceReferences.AddReferencesFromJson(
		myFolderPath.AppendPath("SceneResourceRefs.json")))
	{
		LOG_ERROR(LogType::Resource) << "Failed to load resource refs: SceneResourceRefs.json";
	}

	return true;
}
