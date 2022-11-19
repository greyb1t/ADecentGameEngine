#pragma once

class GameObject;

class GeneralEventListener
{
public:
	virtual void OnEvent(GameObject* aGameObject);
};