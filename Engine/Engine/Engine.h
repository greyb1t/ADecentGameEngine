#pragma once

#include "TimeStamp.h"
#include "Common\LinearAllocator.h"
#include "Engine\Renderer\CursorMode.h"
#include "Scene\SceneHandle.h"

class GameObject;
class ThreadPool;

namespace Engine
{
	class GraphicsEngine;
	class SpotLight;
	class ModelInstance;

	class ModelFactory;
	class CameraFactory;
	class RendererScene;
	class Paths;
	class LightFactory;
	class FullscreenTextureFactory;
	class Camera;
	class ParticleEmitterFactory;
	class TextFactory;
	class ResourceThreadContext;
	class FogManager;
	struct RenderingStatistics;
}

namespace Common
{
	class Timer;
	class InputManager;
}

class FreeCameraController;
class FileWatcher;
class JsonManager;
struct EngineSettings;

class Physics;

namespace Engine
{
	struct Viewport
	{
		// The window position
		Vec2f myPosition;

		// The window size
		Vec2f myRendererSize;

		// The relative position, meaning top left is 0,0
		Vec2f myRelativeMousePosition;
		Vec2f myRelativeNormalizedMousePosition;
	};

	class DebugDrawerSettings;
	class ResourceReferences;
	class CameraShakeManager;
	class TimeSystem;
	class PhysXManager;
	class SceneManager;
	class Scene;
	class DebugDrawer;
	class AnimationClipLoader;
	class ResourceManager;
	class CommandHandler;
	class IngameConsole;
	class Editor;

	class Engine
	{
	public:
		Engine();
		~Engine();

		bool Init(const EngineSettings& aEngineSettings);

		void Update();

		LRESULT CALLBACK WndProc(HWND aWindowHandle, UINT aMessage, WPARAM aWparam, LPARAM aLparam);

		SceneManager& GetSceneManager();
		Physics& GetPhysics();
		TimeSystem& GetTimeSystem();
		TextFactory& GetTextFactory();
		GraphicsEngine& GetGraphicsEngine();

		ThreadPool& GetParallelizationThreadPool();

		Common::Timer& GetTimer();

		void SetTimeScale(float);
		float GetTimeScale() const;

		Common::InputManager& GetInputManager();

		CameraFactory& GetCameraFactory();
		CameraShakeManager& GetCameraShakeManager();

		DebugDrawerSettings& GetDebugDrawerSettings();

		FileWatcher& GetFileWatcher();
		JsonManager& GetJsonManager();

		void StartEditor();
		bool IsEditorEnabled() const;
		Editor& GetEditor();
		SceneHandle GetEditorSceneHandle() const;

		ResourceManager& GetResourceManager();

		CommandHandler& GetCommandHandler();

		ResourceReferences& GetEngineResourceRefs();

		Vec2f GetWindowSize() const;
		Vec2f GetRenderingSize() const;
		const Vec2ui& GetTargetSize() const;

		const Viewport& GetViewport() const;

		void SetCursorMode(const CursorMode aCursorMode);
		CursorMode GetCursorMode() const;
		void SetCursorVisiblity(const bool aCursorVisible);
		bool IsCursorVisible() const;

		const Paths& GetPaths() const;

		static void SetInstance(Engine& aEngine);
		static Engine& GetInstance();

	private:
		bool LoadLoggingSettings();
		bool SaveLoggingSettings();
		void DrawLogDebugMenu();
		void DrawRenderingStatsDebugMenu(const TimeStamp aTs);
		void DrawSceneStats(const RenderingStatistics& aStats) const;
		void DebugDrawSystems(const float aDeltaTime);
		void DrawTestingDebugMenu();
		void UpdateViewport();
		void DrawEngineDebugMenu();

	private:
		friend class SceneManager;

		Viewport myViewport;

		float myTimeScale = 1.f;

		float myPrevRenderingFrameMs = 0.f;

		// If something needs to be deleted in a specific order,
		// set the value to the Owned<> to nullptr and it will release the memory

		Owned<SceneManager> mySceneManager = nullptr;
		Owned<Physics> myPhysics = nullptr;
		Owned<TimeSystem> myTimeSystem = nullptr;
		Owned<GraphicsEngine> myGraphicsEngine = nullptr;
		Owned<Camera> myCamera = nullptr;

		// In its Update() it gives draw calls to DebugRendererer, add this into each scene.
		Owned<DebugDrawerSettings> myDebugDrawerSettings = nullptr;

		Owned<TextFactory> myTextFactory = nullptr;

		Owned<Common::Timer> myTimer = nullptr;
		Owned<Common::InputManager> myInputManager = nullptr;

		Owned<FileWatcher> myFileWatcher = nullptr;
		Owned<JsonManager> myJsonManager = nullptr;

		Owned<CameraShakeManager> myCameraShakeManager = nullptr;

		Owned<ResourceManager> myResourceManager = nullptr;

		Owned<CommandHandler> myCommandHandler;
		Owned<IngameConsole> myIngameConsole;

		Owned<ResourceReferences> myGlobalResourceReferences;
		Owned<ResourceReferences> myEngineResourceRefs;

		// Only to be used as a temporary parallellization
		Owned<ThreadPool> myParallelizationThreadPool;
		// static thread_local LinearAllocator myAllocator;

		Owned<Paths> myPaths;

		Shared<Editor> myEditor;
		SceneHandle myEditorSceneHandle;
		int RigidStaticComponent;

		Engine(const Engine&) = delete;
		void operator=(const Engine&) = delete;

		inline static Engine* myInstance = nullptr;
		void UpdateTimings();
	};

	Shared<Scene> GetScene();
}

inline Engine::Engine& GetEngine()
{
	return Engine::Engine::GetInstance();
}