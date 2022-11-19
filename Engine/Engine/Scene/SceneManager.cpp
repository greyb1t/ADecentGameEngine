#include "pch.h"
#include "SceneManager.h"
#include "Scene.h"
#include "Engine/Engine.h"
#include "Engine/Renderer/GraphicsEngine.h"
#include "Engine/TimeSystem/TimeSystem.h"
#include "Engine/Editor/Editor.h"
#include "Engine/Scene/FolderScene.h"
#include "Engine/Shortcuts.h"
#include "Engine/ResourceManagement/ResourceManager.h"
#include "Engine/GameObject/GameObject.h"

Engine::SceneManager::SceneManager()
	: myThreadPool(1)
{
	// In the case that someone modifies a gameobject prefab
	// in Scene1 and adds a resource onto that prefab.
	// Then when loading Scene2 that has an instance of that prefab
	// tries to load the resource because it has not been written
	// into its ResourceRef list. It fails loading the resource because
	// its thread does not have an entry in ResourceManager::myContext
	// therefore we create that context here
	GResourceManager->CreateThreadContext(myThreadPool.GetThreadIds()[0]);
}

Engine::SceneManager::~SceneManager()
{
}

void Engine::SceneManager::UpdateAllScenes(const float aTimeScale)
{
	// NOTE(filip): Must sort them to ensure they are updated in their correct order because
	// the editor scene must be run before its children scenes
	std::vector<std::pair<SceneHandle, Shared<Scene>>> scenesToIterate;
	scenesToIterate.insert(scenesToIterate.begin(), myScenes.begin(), myScenes.end());

	std::sort(scenesToIterate.begin(), scenesToIterate.end(),
		[](const auto& aLeft, const auto& aRight)
		{
			return aLeft.first < aRight.first;
		});

	for (const auto& [_, scene] : scenesToIterate)
	{
		ZoneNamedN(zone1, "SceneIteration", true);
		ZoneTextV(zone1, scene->GetName().c_str(), scene->GetName().size());

		if (!scene->IsEnabled())
		{
			continue;
		}

		mySceneBeingUpdated = scene.get();

		GScene = scene.get();
		GDebugDrawer = &scene->GetRendererScene().GetDebugDrawer();

		const bool ready = scene->IsReady();

		if (!scene->HasBeenInited())
		{
			assert(false);
			LOG_ERROR(LogType::Engine) << "Scene has not been inited";
			return;
		}

		if (ready)
		{
			// Scene/Game Update
			scene->UpdateSystems(eEngineOrder::EARLY_UPDATE);
		}

		if (ready)
		{
			//GetEngine().GetGraphicsEngine().ImGuiNewFrame();
		}

		scene->UpdateInternal(Time::DeltaTime);

		if (ready)
		{
			scene->UpdateSystems(eEngineOrder::UPDATE);

			if (scene->IsSceneBeingEdited())
			{
				scene->myGameObjectManager.EditorUpdate();
			}

			bool shouldFetchPhysics = false;
			if (aTimeScale > 0.00001f && Time::DeltaTime > 0.0f)
			{
				scene->UpdateSystems(eEngineOrder::EARLY_PHYSICS);
				if (!scene->myIsEditorScene && scene->myShouldUpdate && dynamic_cast<Editor*>(scene.get()) == nullptr)
				{
					ZoneNamedN(zone2, "SimulatePhysics", true);
					scene->SimulatePhysics(Time::DeltaTime);
					shouldFetchPhysics = true;
					//scene->FetchPhysics(Time::DeltaTime);
					//scene->UpdateSystems(eEngineOrder::LATE_PHYSICS);
				}
			}

			scene->LateUpdateInternal(Time::DeltaTime);
			scene->UpdateSystems(eEngineOrder::LATE_UPDATE);
			scene->UpdateSystems(eEngineOrder::EARLY_RENDER);

			scene->Render();

			

			scene->OnImguiRenderInternal();

			scene->BeginRenderReal(GetEngine().GetGraphicsEngine());

			//GetEngine().GetGraphicsEngine().ImGuiRenderFrame();

			scene->EndRenderReal(GetEngine().GetGraphicsEngine());

			if (shouldFetchPhysics)
			{
				ZoneNamedN(zone2, "Fetch Physics", true);
				scene->FetchPhysics(Time::DeltaTime);
				scene->UpdateSystems(eEngineOrder::LATE_PHYSICS);
			}
		}
	}

	// We must reset these because if someone relied on using these after
	// we stopped iterating the scenes, the behaviour is undefined
	// because the globals have the values of the last scene
	GScene = nullptr;
	GDebugDrawer = nullptr;

	mySceneBeingUpdated = nullptr;

	// Update jobs
	if (!myCurrentJob.has_value())
	{
		if (!myLoadJobs.empty())
		{
			// Take next job
			myCurrentJob = std::move(myLoadJobs.front());
			myLoadJobs.pop();

			Shared<Scene> sceneRef = myCurrentJob->myScene;

			myCurrentJob->myJobFuture = myThreadPool.PushTaskFuture(
				[sceneRef]() -> Shared<Scene>
				{
					if (!sceneRef->Init())
					{
						return nullptr;
					}

					sceneRef->StartLoading();

					while (!sceneRef->IsReady())
					{
						sceneRef->UpdateInternal(Time::DeltaTime);
					}
					return sceneRef;
				});
		}
	}
	else
	{
		if (myCurrentJob->myJobFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
		{
			try
			{
				Shared<Scene> loadedScene = myCurrentJob->myJobFuture.get();

				assert(loadedScene == myCurrentJob->myScene);
				assert(myCurrentJob->myScene->IsReady());

				myScenes[myCurrentJob->myHandle] = loadedScene;

				// If this is the only scene, make it active
				if (myMainScene == nullptr)
				{
					SetMainScene(myCurrentJob->myHandle);
				}

				LOG_INFO(LogType::Engine) << "Threadly finished loading scene " << myCurrentJob->myHandle;
			}
			catch (const std::exception& e)
			{
				LOG_ERROR(LogType::Engine) << "Loading scene exception: " << e.what() << ", valid: " << myCurrentJob->myJobFuture.valid();
			}

			myCurrentJob = std::nullopt;
		}
	}

	if (SceneExists(myNextScene) && myNextScene.IsValid())
	{
		if (myClearOtherScenesWhenFinished)
		{
			Shared<Scene> dick = myScenes[myNextScene];
			myScenes.clear();
			myScenes[myNextScene] = dick;
			myClearOtherScenesWhenFinished = false;
		}

		SetMainScene(myNextScene);
		myNextScene = static_cast<SceneHandle>(-1);
	}

	for (const auto& sceneToRemove : myScenesToRemove)
	{
		RemoveScene(sceneToRemove);
	}

	myScenesToRemove.clear();
}

Engine::SceneHandle Engine::SceneManager::AddScene(Shared<Scene> aScene)
{
	SceneHandle handle = CreateNewHandle();

	aScene->StartLoading();
	myScenes[handle] = aScene;

	// If this is the only scene, make it active
	if (myMainScene == nullptr)
	{
		SetMainScene(handle);
	}

	return handle;
}

Shared<Engine::Scene> Engine::SceneManager::RemoveScene(const SceneHandle aHandle)
{
	if (myScenes.find(aHandle) == myScenes.end())
	{
		return nullptr;
	}

	Shared<Scene> scene = myScenes.at(aHandle);
	myScenes.erase(aHandle);
	return scene;
}

void Engine::SceneManager::SetMainScene(const SceneHandle aHandle)
{
	GetEngine().GetTimeSystem().ClearSceneTimers();
	GetEngine().SetTimeScale(1.f);

	if (aHandle.IsValid())
	{
		myScenes[aHandle]->OnSceneSetAsMain();
		myMainScene = myScenes[aHandle];
	}
}

void Engine::SceneManager::SetNextScene(const SceneHandle aHandle, const bool aClearOtherScenesWhenFinished)
{
	myClearOtherScenesWhenFinished = aClearOtherScenesWhenFinished;
	myNextScene = aHandle;
}

Engine::Scene* Engine::SceneManager::GetSceneBeingUpdatedNow()
{
	return mySceneBeingUpdated;
}

const std::optional<Engine::SceneManager::LoadJob>& Engine::SceneManager::GetCurrentLoadJob() const
{
	return myCurrentJob;
}

Shared<Engine::Scene> Engine::SceneManager::GetMainScene()
{
	return myMainScene;
}

void Engine::SceneManager::PushLoadJob(LoadJob&& aLoadJob)
{
	myLoadJobs.push(std::move(aLoadJob));
}

Engine::SceneHandle Engine::SceneManager::CreateNewHandle()
{
	return myHandleCounter++;
}

Engine::SceneHandle Engine::SceneManager::LoadSceneAsync(const std::string& aFolderPath)
{
	SceneHandle handle = CreateNewHandle();

	auto scene = MakeShared<FolderScene>();
	scene->SetFolderPath(aFolderPath);
	scene->SetName(aFolderPath);
	if (!scene->Init())
	{
		return false;
	}
	LoadJob job;
	job.myScene = scene;
	job.myHandle = handle;
	PushLoadJob(std::move(job));

	return handle;
}

Engine::SceneHandle Engine::SceneManager::LoadSceneAsync(Shared<Scene> aScene)
{
	SceneHandle handle = CreateNewHandle();

	SceneManager::LoadJob job;
	job.myHandle = handle;
	job.myScene = aScene;

	PushLoadJob(std::move(job));

	return handle;
}

const std::unordered_map<Engine::SceneHandle, Shared<Engine::Scene>>& Engine::SceneManager::GetScenes() const
{
	return myScenes;
}

bool Engine::SceneManager::SceneExists(const SceneHandle aHandle) const
{
	return myScenes.find(aHandle) != myScenes.end();
}
