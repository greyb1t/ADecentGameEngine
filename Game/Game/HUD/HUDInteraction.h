#pragma once
#include "Engine/GameObject/Components/Component.h"

namespace Engine
{
	class TextComponent;
	class SpriteComponent;
}

class HUDInteraction : public Component
{
public:
	COMPONENT(HUDInteraction, "HUD Interaction");

	HUDInteraction() = default;
	HUDInteraction(GameObject* aGameObject);
	~HUDInteraction();

	void Reflect(Engine::Reflector& aReflector) override;

	void Start();

	void Execute(Engine::eEngineOrder aOrder) override;

	//For ex. Items
	void SetInteractionName(const std::string& aName);
	void SetText(const std::string& aText);

	void ShowDesc(bool aShouldShow);

	void Decline();

private:
	Engine::TextComponent* myNameText = nullptr;
	std::string myInteractionText = "E - ";

	Engine::SpriteComponent* myBackground = nullptr;

	float myDeclineTime = 0.3f;
	float myDeclineTimer = 0.0f;
};
