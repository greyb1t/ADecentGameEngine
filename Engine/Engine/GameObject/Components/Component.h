#pragma once

#include "Engine/EngineOrder.h"
#include "Engine/Reflection/Reflectable.h"
#include "Engine/GameObject/Transform.h"
#include "Engine/GameObject/ComponentSystem/ComponentFactory.h"
#include "Engine/GameObject/SingletonComponent.h"

class TimeStamp;
class GameObject;

namespace Engine
{
	class ComponentHandler;
	class ComponentSystem;
	enum eEngineOrder;
	class ComponentFactory;
}

class Component : public Engine::Reflectable
{
public:
	Component() = default;

	Component(GameObject* aGameObject);
	virtual ~Component();

	virtual void OnConstruct() {}

	// Called just before Start(), this is like a pre-init
	virtual void Awake() { }

	// This is called after the GameObject has called Start()
	// Used to call GetComponent() and save the components
	virtual void Start() { }

	virtual void OnEnable() { }
	virtual void OnDisable() { }

	// This will only run in runtime, not in editor
	virtual void Execute(Engine::eEngineOrder aOrder) { }

	virtual void EditorExecute() { }
	// Called when this gameobject is selected in scene hierarchy
	virtual void EditorExecuteSelected() { }

	virtual void ExecuteParallelized(Engine::eEngineOrder aOrder) { }

	// Is called after Execute() and ExecuteParallelized() has run
	virtual void PostExecute() { }

	// This will run when in editor mode and runtime
	virtual void Render() { }

	virtual void RenderParallelized() { }

	bool IsActive() const;
	virtual void SetActive(bool aValue);

	bool HasStarted() const;

	GameObject* GetGameObject();
	const GameObject* GetGameObject() const;

	Transform& GetTransform();
	const Transform& GetTransform() const;

	void Reflect(Engine::Reflector& aReflector) override;

	int GetComponentTypeID() const;

	bool IsDestroyed() const;
	bool IsEarlyDestroy() const;
	void Destroy();

	// This should be implemented by the derrived component and call its copy constructor
	virtual Component* Clone() const {
		return nullptr;
	}

private:
	void InternalStart();
	void InternalAwake();

	// Should hopefully not be required to use
	virtual void ChangeParentGameObject(GameObject& aNewGameObject);

protected:
	friend class Engine::ComponentSystem;
	friend class Engine::ComponentFactory;
	friend class Engine::ComponentHandler;
	friend class InfoRigidBodyComponent;
	friend class InfoRigidStaticComponent;
	friend class GameObject;
	int myID = -1;
	bool myIsActive = true;
	bool myIsDestroyed = false;
	GameObject* myGameObject = nullptr;

	bool myHasAwaked = false;
	bool myHasStarted = false;
};

namespace Engine
{
	class AnimatorComponent;
	class BezierSplineComponent;
	class AudioComponent;
	class CameraComponent;
	class CameraShakeComponent;
	class CharacterControllerComponent;
	class Collider;
	class DecalComponent;
	class EnvironmentLightComponent;
	class FollowAnimatedBoneComponent;
	class FollowBoneComponent;
	class InfoRigidBodyComponent;
	class InfoRigidStaticComponent;
	class LandscapeComponent;
	class LogoFader;
	class MeshComponent;
	class ModelComponent;
	class ParticleEmitterComponent;
	class PointLightComponent;
	class PostProcessComponent;
	class ScriptsComponent;
	class SkyboxComponent;
	class SpotLightComponent;
	class SpriteComponent;
	class TextComponent;
	class TriggerComponent;
	class VFXComponent;
	class VFXSpawnerComponent;
	class VideoComponent;
}
