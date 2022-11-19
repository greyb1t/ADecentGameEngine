#include "pch.h"
#include "MenuComponent.h"

#include "Engine/GameObject/Components/SpriteComponent.h"

void MenuComponent::Start()
{
	UI::AddEvent("EnterButton", [this](GameObject* aButtonObject) { EnterButton(aButtonObject); });
	UI::AddEvent("ExitButton", [this](GameObject* aButtonObject) { ExitButton(aButtonObject); });
}

void MenuComponent::EnterButton(GameObject* aButtonObject)
{
	if (auto sprite = aButtonObject->GetComponent<Engine::SpriteComponent>())
	{
		float a = sprite->GetAlpha();
		sprite->SetColor({ 1, 0, 0, a });
	}
}

void MenuComponent::ExitButton(GameObject* aButtonObject)
{
	if (auto sprite = aButtonObject->GetComponent<Engine::SpriteComponent>())
	{
		float a = sprite->GetAlpha();
		sprite->SetColor({ 1, 1, 1, a });
	}
}
