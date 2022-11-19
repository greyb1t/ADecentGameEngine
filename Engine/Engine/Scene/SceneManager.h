#pragma once

#include "Engine/TimeStamp.h"

#include <optional>
#include <queue>
#include <future>
#include <vector>
#include <xhash>

#include "crude_json.h"
#include "Common/ThreadPool/ThreadPool.h"
#include "SceneHandle.h"

namespace Engine
{
	class GraphicsEngine;
	enum eEngineOrder;
	class Scene;
	class Editor;

	class SceneManager
	{
		enum class State
		{
			StartedChangingScene,
			ChangingScene,
			UpdatingCurrentScene
		};

	public:
		struct LoadJob
		{
			Shared<Scene> myScene;
			std::future<Shared<Scene>> myJobFuture;
			SceneHandle myHandle = static_cast<SceneHandle>(-1);
		};

		SceneManager();
		~SceneManager();

		void UpdateAllScenes(const float aTimeScale);

		// DO NOT USE, DEPRECATED
		SceneHandle AddScene(Shared<Scene> aScene);

		Shared<Scene> RemoveScene(const SceneHandle aHandle);

		void SetMainScene(const SceneHandle aHandle);

		void SetNextScene(const SceneHandle aHandle, const bool aClearOtherScenesWhenFinished = true);

		// Returns the main scene which is rendered to the backbuffer, meaning
		// if in editor, that scene will be returned
		Shared<Scene> GetMainScene();

		SceneHandle LoadSceneAsync(const std::string& aFolderPath);
		SceneHandle LoadSceneAsync(Shared<Scene> aScene);

		const std::unordered_map<SceneHandle, Shared<Scene>>& GetScenes() const;

		bool SceneExists(const SceneHandle aHandle) const;

		// DO NOT USE UNLESS YOU KNOW WHAT YOU'RE DOING
		// SINCE WE CAN HAVE MULTIPLE SCENES AT ANY TIME, THIS RETURNS
		// DIFFERENT SCENES DEPENDING ON WHEN YOU CALL IT
		// Use myGameObject->GetScene() to get the scene in most cases
		Scene* GetSceneBeingUpdatedNow();

		const std::optional<LoadJob>& GetCurrentLoadJob() const;

	private:
		friend class Scene;
		friend class Engine;

		// Starts init and loads the scene on a separate thread
		void PushLoadJob(LoadJob&& aLoadJob);

		SceneHandle CreateNewHandle();

	private:
		uint32_t myHandleCounter = 0;

		// This is the scene being renderered onto the backbuffer
		Shared<Scene> myMainScene;

		std::unordered_map<SceneHandle, Shared<Scene>> myScenes;

		Scene* mySceneBeingUpdated = nullptr;

		State myState = State::ChangingScene;

		std::queue<LoadJob> myLoadJobs;
		std::optional<LoadJob> myCurrentJob;

		ThreadPool myThreadPool;

		SceneHandle myNextScene = -1;
		bool myClearOtherScenesWhenFinished = false;

		std::vector<SceneHandle> myScenesToRemove;
	};
}
