#include "pch.h"
#include "Engine/Engine.h"
#include "Engine/Scene/Scene.h"
#include "GraphNode_LoadScene.h"
#include "Engine/GraphManager/Nodes/Base/GraphNodeInstance.h"
#include "Engine/Scene/SceneManager.h"
#include "Engine/GameObject/GameObject.h"
#include "../Game/Game/Scenes/GameScene.h"

GraphNode_LoadScene::GraphNode_LoadScene()
{
	CreatePin("IN", PinDirection::PinDirection_IN, DataType::Exec);
	CreatePin("Scene Name", PinDirection::PinDirection_IN, DataType::String);
}

int GraphNode_LoadScene::OnExec(class GraphNodeInstance* aNodeInstance)
{
	auto sceneName = GetPinData<std::string>(aNodeInstance, 1);
	Engine::SceneManager::LoadJob job;
	auto scene = MakeShared<Engine::FolderScene>();

	/*
	if (sceneName == "Level1")
	{
		scene = MakeShared<Level1>();
		LoadScene(sceneName, "UEDPIE_0_01_Breakthrough", scene);
	}
	else if (sceneName == "Level2")
	{
		scene = MakeShared<Level2>();
		LoadScene(sceneName, "UEDPIE_0_03_ComputerLab", scene);
	}
	else if (sceneName == "Level3")
	{
		scene = MakeShared<Level3>();
		LoadScene(sceneName, "UEDPIE_0_04_LabExits", scene);
	}
	else if (sceneName == "Level4")
	{
		scene = MakeShared<Level4>();
		LoadScene(sceneName, "UEDPIE_0_05_OfficeGarden", scene);
	}
	else if (sceneName == "Level5")
	{
		scene = MakeShared<Level5>();
		LoadScene(sceneName, "UEDPIE_0_08_GettingOut", scene);
	}
	*/

	// OLD
	/*
	else if (sceneName == "Level6")
	{
		scene = MakeShared<Engine::FolderScene>();
		LoadScene(sceneName, "UEDPIE_0_07_AnotherDay", scene);
	}
	else if (sceneName == "Level7")
	{
		scene = MakeShared<Engine::FolderScene>();
		LoadScene(sceneName, "UEDPIE_0_08_GettingOut", scene);
	}
	*/
	return -1;
}

bool GraphNode_LoadScene::WaitForScene(const std::string& aSceneName)
{
	assert(false && "filip removed this when refactoring, new method required");
	return false;
	//if (GetEngine().GetSceneManager().SceneExists(aSceneName))
	//{
	//	return true;
	//}
	//return false;
}
bool GraphNode_LoadScene::LoadScene(const std::string aSceneName, const std::string aFolderName, Shared<Engine::FolderScene> aEmptyScenePtr)
{
	assert(false && "filip removed this when refactoring, new method required");

	/*
	Engine::SceneManager::LoadJob job;

	aEmptyScenePtr->SetFolderPath("Assets\\Scenes\\" + aFolderName);
	//auto a = "Assets\\Scenes\\" + aFolderName;
	if (!aEmptyScenePtr->Init())
	{
		return false;
	}
	job.mySceneName = aFolderName;
	job.myScene = aEmptyScenePtr;
	GetEngine().GetSceneManager().PushLoadJob(std::move(job));

	//while (!WaitForScene(aSceneName)) {}

	GetEngine().GetSceneManager().SetNextScene(aFolderName);
	*/

	return true;
}
