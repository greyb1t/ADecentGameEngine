#pragma once

#include "Engine/Engine.h"
#include "Engine/LiteTransform.h"
// Emelia was here!

class GameObject;

// TODO: Instead of calculating the world pos/rot/
//  everytime
// Do a pass before each frame and calculate them and then cache them
// only update them if dirty
// better performance, and can be parallellized

namespace Engine
{
	class GameObjectManager;
	class Scene;
	class GameObjectPrefab;
	class GameObjectChildIterator;
}

class Transform : public Engine::LiteTransform
{
public:
	Transform() = default;
	Transform(const Shared<GameObject>& aGameObject);
	~Transform();

	GameObject* GetGameObject();
	const GameObject* GetGameObject() const;
	Shared<GameObject> GetGameObjectShared() const;

	Transform* GetParent() const;
	bool HasParent() const;
	void			SetParent(Transform* aParentTransform);
	void			RemoveParent();

	// Gets the parent highest up in the hierarchy
	Transform* GetTopMostParent();

	// Does not touch position, simply only sets parent
	void			SetParent2(Transform* aParentTransform);
	// Does not touch position, simply only removes parent
	void			RemoveParent2();

	void			Move(const Vec3f& aMovement);
	void			Rotate(const Quatf& aRotation);

	void			SetPosition(const Vec3f& aPos);
	void			SetRotation(const Quatf& aQuaternion);
	void			SetScale(const Vec3f& aScale);
	void			SetScale(const float aScale);
	void			SetScaleX(float aScale);

	Vec3f			GetScale() const;
	const Vec3f		GetPosition() const;
	Quatf			GetRotation() const;

	void			SetPhysical(bool);
	bool			IsPhysical() const;

	void			ResetMovement();
	void			ResetRotate();
	const Vec3f& GetMovement() const;
	const Quatf& GetRotated() const;

	Transform* GetChildByIndex(const int aIndex);
	const std::vector<Transform*>& GetChildren() const;
	bool HasChildren() const;

	GameObject* GetChildByGameObjectName(const std::string& aName, const bool aRecursively = false);

	Vec3f			Forward() const;
	Vec3f			Right() const;
	Vec3f			Up() const;

	Mat4f			GetMatrix() const;
	Mat4f			GetMatrixLocal() const;

	void			LookAt(const Vec3f& aTargetPosition, const Vec3f& aUp = Vec3f(0.0f, 1.0f, 0.0f));
	Quatf			GetLookAtResult(const Vec3f& aTargetPosition, const Vec3f& aUp = Vec3f(0.0f, 1.0f, 0.0f));
	// Used by the Reflection system, do not change, ask filip
	Transform& operator=(const Transform& aRight);

	void ReleaseTransform();

private:
	// This is only for internal use, use AttachVFX for setting up hierarchy
	void AddChild(Transform* aTransform);
	// This is only for internal use, use DetachVFX for setting up hierarchy
	void RemoveChild(Transform* aTransform);

private:
	friend class Engine::GameObjectManager;
	friend class Engine::Scene;
	friend class GameObject;
	friend class Engine::GameObjectPrefab;
	friend class Engine::GameObjectChildIterator;

	friend bool operator!=(const Transform& aLeft, const Transform& aRight);

	Weak<GameObject> myGameObject;

	Transform* myParent = nullptr;

	std::vector<Transform*> myChildren;

	// ____ PHYSICAL BODY VARIABLES ____
	bool myIsPhysicalBody = false;
	Vec3f myMovement;
	Quatf myRotated;
};

// These are used by the Reflection, do not change, ask Filip
bool operator!=(const Transform& aLeft, const Transform& aRight);