#pragma once

#include "Engine/GameObject/Components/Component.h"
#include "Engine/GameObject/GameObject.h"

#include "Engine/UIEventManager/UIEventManager.h"

#include "MenuHandler.h"

using UI = UIEventManager;

class MenuComponent : public Component
{
public:
	MenuComponent() = default;
	~MenuComponent() = default;

	void Start() override;

protected:
	MenuHandler* myMenuHandler = nullptr;

private:
	friend MenuHandler;
	
	void EnterButton(GameObject* aButtonObject);
	void ExitButton(GameObject* aButtonObject);

private:

};
