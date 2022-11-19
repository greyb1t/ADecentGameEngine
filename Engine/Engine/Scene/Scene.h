#pragma once

#include "Engine/ResourceReferences.h"
#include "Engine/TimeStamp.h"
#include "Physics/PhysicsScene.h"
#include "Physics/OverlapHitInternal.h"
#include "Engine/GameObject/Tag.h"
#include "Engine/EngineOrder.h"

#include "Engine/GameObject/GameObjectManager.h"
#include "Engine/Renderer/Scene/RendererScene.h"
#include "Engine/Renderer/Camera/Camera.h"
#include "Engine/GameObject/SingletonComponent.h"

class GameObject;
namespace Engine
{
	class GraphicsEngine;
}

namespace Engine
{
	class Scene;
	class EnvironmentLight;
	class OrthographicCamera;
	struct FogData;
	class FolderScene;
	class ImguiReflector;
}

struct RayCastHit : RayCastHitInternal
{
	RayCastHit() = default;
	RayCastHit(const RayCastHitInternal& in)
		: RayCastHitInternal(in)
	{
	}

	GameObject* GameObject = nullptr;
	int			UUID = -1;
};

struct OverlapHit : OverlapHitInternal
{
	OverlapHit() = default;
	OverlapHit(const OverlapHitInternal& in)
		: OverlapHitInternal(in)
	{

	}

	GameObject* GameObject = nullptr;
	int UUID = -1;
};


namespace Engine
{
	class Entity;
	class EnvironmentLight;
	class ComponentSystem;
	class ResourceReferences;
	class GameObjectManager;

	class Scene : public PhysicsScene, public Observer
	{
	public:
		enum class InitState
		{
			LoadResources = 1,
			LoadScene, // our scene from the editor
			LoadWorld, // the world from unreal
			RuntimeInit,
			Count,
			None
		};

		enum class State
		{
			NotReady,
			Ready,
			Failed
		};

		enum class Result
		{
			NotFinished,
			Succeeded,
			Failed
		};

		Scene();
		virtual ~Scene();
		Scene(const Scene& aOther);
		Scene& operator=(const Scene& aOther);

		void StartLoading();

		// Load our scene that was saved from the editor
		virtual Result LoadScene() { return Result::Succeeded; }

		// Returns true when finished
		virtual Result LoadWorld() { return Result::Succeeded; }

		// Returns true when finished
		virtual Result OnRuntimeInit();

		virtual bool Init();

		void UpdateInternal(TimeStamp ts);
		virtual void Update(TimeStamp ts);

		void LateUpdateInternal(TimeStamp ts);
		virtual void LateUpdate(TimeStamp ts);

		virtual void CreateResourceReferences();

		virtual void OnSceneSetAsMain();

		void OnImguiRenderInternal();
		virtual void OnImguiRender();

		void DrawDebugMenuInternal();
		virtual void DrawDebugMenu();

		virtual void UpdateSystems(eEngineOrder aOrder);
		virtual void Render();

		// Can override this to make scene draw another scene in itself
		// used in editor
		virtual void BeginRenderReal(GraphicsEngine& aGraphicsEngine);
		virtual void EndRenderReal(GraphicsEngine& aGraphicsEngine);

		// Adds a GameObject to the current scene
		// NOTE(filip): moving this outside of here, caused BAAAAAD errors lol?
		template <class T, class... Args>
		T* AddGameObject(Args&&... args)
		{
			T* g = this->myGameObjectManager.AddGameObject<T, Args...>(std::forward<Args>(args)...);
			//static_cast<GameObject*>(g)->myScene = this;
			return g;
		}

		const std::vector<Shared<GameObject>>& GetGameObjects() const;
		GameObject* FindGameObject(int aUUID);
		GameObject* FindGameObject(const std::string& aName) const;
		std::vector<GameObject*> FindGameObjects(eTag);
		int CountGameObjectsWithName(const std::string& aName) const;

		bool ScreenToWorld(const Vec2ui& aScreenPosition, unsigned int aLayerMask, Vec3f& outPosition);
		Vec3f ScreenToWorldDirection(const Vec2ui& aScreenPosition);

		bool RayCast(const Vec3f& aOrigin, const Vec3f& aDirection, float aRange, LayerMask aLayerMask, RayCastHit& outHit, bool aFindGameObject = true);
		bool RayCastAll(const Vec3f& aOrigin, const Vec3f& aDirection, float aRange, LayerMask aLayerMask, std::vector<RayCastHit>& outHit, bool aFindGameObject = true);

		// TODO: implement this!
		//bool GeometryCastAll(const physx::PxGeometry& aGeometry, const Vec3f& aPosition, const Quatf& aQuaternion, LayerMask aLayerMask, std::vector<OverlapHitInternal>& outHits);
		bool BoxCastAll(const Vec3f& aPosition, const Quatf& aQuaternion, const Vec3f& aHalfSize, LayerMask aLayerMask, std::vector<OverlapHit>& outHits, bool aFindGameObject = true);
		bool SphereCastAll(const Vec3f& aPosition, float aRadius, LayerMask aLayerMask, std::vector<OverlapHit>& outHits, bool aFindGameObject = true);
		bool GeometryCastAll(const physx::PxGeometry& aGeometry, const Vec3f& aPosition, const Quatf& aQuat, LayerMask aLayerMask, std::vector<OverlapHit>& outHits, bool aFindGameObjects = true);

		GameObject* GetMainCameraGameObject();
		Camera& GetMainCamera();

		void SetName(const std::string& aName);
		virtual std::string GetName() const { return myName; }

		RendererScene& GetRendererScene();
		RendererScene& GetUIRendererScene();
		ComponentSystem& GetComponentSystem();

		bool HasBeenInited() const;

		bool IsReady() const;

		// You must delete the Scene
		virtual Scene* Clone() const { return nullptr; }

		// Only used to be able for the editor to clone the scene
		// before actually creating the gameobjects from Start code
		void SetOnLoadedWorldCallback(std::function<void()> aOnLoadedWorld);

		void SetMainCamera(GameObject* aGameObject);

		InitState GetInitState() const;

		bool CreateDefaultEnvironmentLight();
		void CreateDefaultCamera();

		ResourceReferences& GetResourceReferences();

		// EDITOR FUNCTIONS
		void SetIsEditorScene(const bool aIsEditorScene);
		bool IsEditorScene() const;

		void SetShouldUpdate(const bool aShouldUpdate);
		bool ShouldUpdate() const;

		Shared<GameObject> CreateGameObject();

		virtual void Receive(const EventType aEventType, const std::any& aValue) override;

		bool IsEnabled() const;
		void SetIsEnabled(const bool aIsEnabled);

		bool IsSceneBeingEdited() const;

		GameObject* GetEnvironmentLight();

		void Reload();

		float GetLoadingPercent() const;

	private:
		void UpdateNotFinished(TimeStamp ts);
		void UpdateSingletonGameObjectsRefs();

		// Copies the given gameobject manager into this scene
		void CloneGameObjects(const GameObjectManager& aGameObjectManager);

	protected:
		friend class Entity;
		friend class SceneManager;
		friend class ComponentSystems;
		friend class SceneHierarchyWindow;
		friend class Editor;
		friend class AddGameObjectCommand;
		friend class RemoveGameObjectCommand;
		friend class GameObjectPrefab;
		friend class InspectorWindow;
		friend class AssetBrowserWindow;
		friend class MoveGameObjectCommand;
		friend class InspectableSelection;
		friend class GameObject;
		friend class InspectableGameObjectPrefab;
		friend class InspectGameObject;
		friend class ImguiReflector;

	protected:
		RendererScene myRenderScene;

		// omg, this is an abomination
		RendererScene myUIRenderScene;
		Camera myUICamera;

		GameObjectManager myGameObjectManager;

		GameObject* myCameraGameObject = nullptr;
		GameObject* myEnvironmentLightGameObject = nullptr;

		// Renderer::EnvironmentLight myEnvironmentLight;
		bool myEnvironmentLightFollowCamera = false;

		ResourceReferences myResourceReferences;

	private:
		bool myIsEnabled = true;

		bool myHasBeenInited = false;

		// Must be atomic because used from the outside when waiting for the 
		// scene to be finished loading on another thread
		std::atomic<State> myState = State::NotReady;
		InitState myInitState = InitState::LoadResources;

		float myLoadingPercent = 0.f;

		std::function<void()> myOnLoadedWorld = nullptr;

		// eww
		bool myIsEditorScene = false;
		// eww x2
		bool mySceneBeingEdited = false;

		// If we should call Update() on all gameobjects
		// used by editor to pause the scene
		bool myShouldUpdate = true;

		// A cloned version of this scene that is unchanged
		std::shared_ptr<FolderScene> myClonedScene;

		std::string myName;
	};
}