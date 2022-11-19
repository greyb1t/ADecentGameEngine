#pragma once

#include "Engine/GameObject/Components/Component.h"

class LookAt : public Component
{
	COMPONENT(LookAt, "LookAt");
public:

	void Start() override;
	void Execute(Engine::eEngineOrder aOrder) override;

	void Reflect(Engine::Reflector& aReflector) override;

private:
	Engine::GameObjectRef myLookAtObjectRef;
	GameObject* myLookAtObject = nullptr;

	bool myLookAtPlayer = false;

};

