#pragma once
#include "Component.h"
#include "Physics/RigidStatic.h"

namespace Engine
{
class RigidStaticComponent :
    public Component, public RigidStatic
{
public:
	RigidStaticComponent(GameObject* aGameObject);
	~RigidStaticComponent();
	
	void Start() override;
	void Execute(eEngineOrder aOrder) override;

	Component* Clone() const override;

	void SetOnCollisionListener(const std::function<void(GameObject*)>& aFunction);
	void SetOnEnterCollisionListener(const std::function<void(GameObject*)>& aFunction);
	void SetOnExitCollisionListener(const std::function<void(GameObject*)>& aFunction);

private:
	void DebugDraw();

	std::function<void(GameObject*)> myOnCollisionEnterObserver;
	std::function<void(GameObject*)> myOnCollisionObserver;
	std::function<void(GameObject*)> myOnCollisionExitObserver;
};
}