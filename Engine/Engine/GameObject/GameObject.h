#pragma once

#include "Tag.h"
#include "Transform.h"
#include "Engine/Scene/Scene.h"
#include "Components/Component.h"

#include "ComponentSystem/ComponentSystem.h"
#include "Physics/Types.h"
#include "SingletonComponent.h"
#include "Engine/ResourceManagement/ResourceRef.h"
#include "Engine/UUID.h"

class Scene;

namespace Engine
{
	class SceneWindow;
	class GameObjectPrefab;
	class InspectorWindow;
	class Scene;
	class GameObjectPrefab;
	class GameObjectPrefabReferencesReflector;
	class RemoveGameObjectCommand;
	class SceneSerializerJson;
	class GameObjectChildIterator;
	class InspectableSelection;
	class InspectGameObject;
	class Editor;
	class AddComponentCommand;
	class RemoveComponentCommand;
	class ComponentHandler;
	class SceneManager;
	class GameObjectPrefabResource;

	class GameObjectManager;
	class Scene;
	class InspectorWindow;
	class SceneSerializerJson;
	class GameObjectPrefab;
	class RemoveGameObjectCommand;
	class AddGameObjectCommand;
	class GameObjectChildIterator;
	class InspectableSelection;
	class InspectableGameObjectPrefab;
	class InspectGameObject;
	class SceneHierarchyWindow;
	class Editor;
	class AddComponentCommand;
	class RemoveComponentCommand;
	class ComponentHandler;
	class GameObjectPrefabResource;
	class ImguiReflector;
}

class GameObject : public Engine::Reflectable
{
public:
	GameObject();
	virtual ~GameObject();

	GameObject(const GameObject& aOther) = default;

	// Do not call manually
	virtual void Start() { }
	virtual void Awake() { }

	// This returns a Weak_ptr of the gameobjects shared_ptr
	Weak<GameObject> GetWeak() const;

	template <typename T>
	Weak<T> GetWeak() const;

	// Called when GameObject is destroyed (this is not a constructor)
	// Return True MEANS that it will NOT remove the instance from the scene.
	virtual bool OnDestroy();

	void Destroy();
	bool IsDestroyed() const;

	void SetActive(const bool aIsActive);
	bool IsActive() const;

	void SetTag(eTag aTag);
	eTag GetTag() const;

	/**
	 * \brief Sets the layers that the Physics bodies connected to the object has
	 * Example:
	 * (eLayer::Grenade (What we are), eLayer::Ground | eLayer::Wall (What we physically interact with, bounce),
	 * eLayer::Enemy (What we want to move through but get notified about)
	 *
	 *
	 * \n INFO:  | between two layers will create a combined layer with both
	 * \param aLayer The Objects layer
	 * \param aPhysicalLayerMask What we physically interact with
	 * \param aDetectionLayerMask What we will be notified on interaction with
	 */
	void		SetLayers(Layer aLayer, LayerMask aPhysicalLayerMask = eLayer::DEFAULT, LayerMask aDetectionLayerMask = eLayer::NONE);
	void		SetLayer(Layer aLayer);
	void		SetPhysicalLayer(LayerMask aPhysicalLayerMask);
	void		SetDetectionLayer(LayerMask aDetectionLayerMask);

	// The layer our gameobject is
	LayerMask	GetLayer() const;
	// LayerMask that is used for which objects we will physically interact with
	LayerMask	GetPhysicalLayerMask() const;
	// LayerMask that is used for which layers we are notified on interaction with
	LayerMask	GetDetectionLayerMask() const;


	void SetName(const std::string& aName);
	const std::string& GetName() const;

	Transform& GetTransform();
	void SetTransform(const Transform& aTransform);
	void SetTransform(const Mat4f& aMatrix4);
	const Transform& GetTransform() const;

	template <class T, class... Args>
	T* AddComponent(Args&&...);

	template <class T>
	T* GetComponent();

	template <class T>
	T* FindComponent();

	template <class T>
	T* GetSingletonComponent();

	Component* GetComponentByTypeId(const int aTypeId);

	Engine::UUID GetUUID() const;
	Engine::Scene* GetScene() const;

	void Reflect(Engine::Reflector& aReflector) override;

	void SetIsFromUnreal(const bool aIsFromUnreal);
	bool IsFromUnreal() const;

	bool IsChildOf(GameObject& aGameObject);

	const GameObjectPrefabRef& GetPrefab() const;
private:
	void SetScene(Engine::Scene* aScene);

	// Returns true if changed
	bool ReflectPrefab(
		Engine::Reflector& aReflector,
		const std::unordered_map<std::string, Shared<Engine::GameObjectPrefab>>& aPrefabGameObjectStates);

	// Reflects all childrens and all components
	void ReflectChildrenAndComponents(Engine::Reflector& aReflector);

	Shared<GameObject> Clone(
		Engine::GameObjectManager& aGameObjectManager,
		const bool aGenerateNewUUID,
		const bool aRecursively = true);

	Shared<GameObject> CloneAsPrefabInstance(
		Engine::GameObjectManager& aGameObjectManager,
		const bool aGenerateNewUUID);

	Shared<GameObject> CloneInternal(
		Engine::GameObjectManager& aGameObjectManager,
		GameObject* aParent,
		const bool aGenerateNewUUID,
		const bool aIsPrefabInstance,
		const bool aRecursively);

	// Gesus this is bad, this is a must due to the Transform member variable
	// having a shared ptr to its parent gameobject
	// it must have a ptr so we are able to retrieve the Shared<GameObject> while
	// recursively iterating the child tree
	void InitTransform(const Shared<GameObject>& aThisGameObjectAsShared);

	void InternalAwake();
	void InternalStart();

	// For editor use
	virtual void AddExistingComponent(Component* aComponent);
	virtual void RemoveComponentNoFree(Component* aComponent);

	void RemoveComponent(Component* aComponent);
	void RemoveComponentInternal(Component* aComponent);

	// Does a dynamic cast, only to be used in editor
	bool IsPrefabInstance() const;

	GameObject* GetPrefabGameObject();

	void SyncWithPrefab(
		GameObject* aBeforeChangedPrefabGameObject,
		GameObject* aAfterChangePrefabGameObject);

	// Updates the whole gameobject, components and its children with the prefab gameobject
	void SyncWithPrefabFull(
		GameObject* aBeforeChangedPrefabGameObject,
		GameObject* aAfterChangePrefabGameObject);

	int GetPrefabGameObjectID() const;

	GameObject* FindChildByPrefabGameObjectID(const int aPrefabGameObjectID);
	GameObject* FindChildByPrefabGameObjectIDIncludingMe(const int aPrefabGameObjectID);

	void ReflectLayerImgui(Engine::Reflector& aReflector, const std::string& aName, LayerMask& aLayerMask);

	const static inline std::string ourReflectNameStr = "Name";
	const static inline std::string ourReflectUUIDStr = "UUID";
	const static inline std::string ourReflectIsActiveStr = "Is Active";
	const static inline std::string ourReflectIsFromUnrealStr = "Is From Unreal";
	const static inline std::string ourReflectTransformStr = "Transform";
	const static inline std::string ourReflectPrefabStr = "Prefab";
	const static inline std::string ourReflectPrefabGameObjectIDStr = "PrefabGameObjectID";
	const static inline std::string ourReflectTagStr = "Tag";
	const static inline std::string ourReflectLayerMask = "Layer";
	const static inline std::string ourReflectPhysicalLayerMask = "Physical Layers";
	const static inline std::string ourReflectDetectionLayerMask = "Detection Layers";

protected:
	friend class Engine::GameObjectManager;
	friend class Engine::Scene;
	friend class Engine::InspectorWindow;
	friend class Engine::SceneSerializerJson;
	friend class Engine::GameObjectPrefab;
	friend class Engine::RemoveGameObjectCommand;
	friend class Engine::AddGameObjectCommand;
	friend class Engine::GameObjectChildIterator;
	friend class Engine::InspectableSelection;
	friend class Engine::InspectableGameObjectPrefab;
	friend class Engine::InspectGameObject;
	friend class Engine::SceneHierarchyWindow;
	friend class Engine::Editor;
	friend class Engine::AddComponentCommand;
	friend class Engine::RemoveComponentCommand;
	friend class Engine::ComponentHandler;
	friend class Engine::GameObjectPrefabResource;
	friend class Engine::ImguiReflector;
	friend class Engine::SceneManager;
	friend class Engine::SceneWindow;
	friend class Engine::GameObjectPrefabReferencesReflector;

	virtual void Update(float aDeltaTime);
	virtual void Render();

protected:
	// int myUUID = 0;
	Engine::UUID myUUID;

	Engine::Scene* myScene = nullptr;
	Weak<GameObject> myWeakPtr;

	bool			myIsActive = true;
	std::string		myName = "GameObject";
	eTag			myTag = eTag::DEFAULT;
	LayerMask		myLayer = eLayer::Enum::DEFAULT;
	LayerMask		myPhysicalLayerMask = eLayer::Enum::DEFAULT;
	LayerMask		myDetectionLayerMask = eLayer::Enum::NONE;

	Transform myTransform;

	bool myIsDestroyed = false;

	// NOTE(filip): ugly fix to prevent saving and being able to
	// edit gameobjects that were loaded from unreal
	bool myIsFromUnreal = false;

	bool myHasAwaked = false;
	bool myHasStarted = false;

private:
	std::vector<Component*> myComponents;

	// Prefab stuff
	GameObjectPrefabRef myPrefab;

	GameObject* myPrefabGameObject = nullptr;

	// A ID that represents the respective gameobject in the 
	// prefab. E.g. Child1 has ID 1, means other instances of
	// that prefab, that specific child also has ID 1
	// Used when syncing the gameobject prefab hierarchy of all
	// prefab instances
	int myPrefabGameObjectID = 0;
};

template <typename T>
Weak<T> GameObject::GetWeak() const
{
	return std::static_pointer_cast<T>(myWeakPtr.lock());
}

template <class T, class... Args>
T* GameObject::AddComponent(Args&&... args)
{
	assert(myScene && "Error Scene is not allowed to be invalid!");

	T* component
		= myScene->GetComponentSystem().AddComponent<T>(this, std::forward<Args&&>(args)...);
	myComponents.emplace_back(component);

	// if the gameobject has already called Start()
	// then Start the component immediately, otherwise
	// Start() would be called from the GameObject::Start()
	if (myHasStarted)
	{
		component->InternalStart();
	}

	return component;
}

template <class T>
T* GameObject::GetComponent()
{
	// Remake this to use ComponentSyste
	for (int i = 0; i < myComponents.size(); i++)
	{
		if (T* component = dynamic_cast<T*>(myComponents[i]))
		{
			return component;
		}
	}

	return nullptr;
}

template <class T>
T* GameObject::FindComponent()
{
	T* component = GetComponent<T>();
	if (component)
		return component;

	int i = 0;
	const auto& children = myTransform.GetChildren();
	while (!component && i < children.size())
	{
		component = children[i]->GetGameObject()->GetComponent<T>();
		++i;
	}

	return component;
}

template <class T>
T* GameObject::GetSingletonComponent()
{
	const int id = Engine::SingletonComponentFactory::GetInstance().GetSingletonComponentId<T>();

	auto& entry = myScene->myGameObjectManager.mySingletonComponentEntries[id];

	return static_cast<T*>(entry.myComponent);
}
