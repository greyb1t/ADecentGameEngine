#pragma once
#include "Engine/GameObject/Components/Component.h"

namespace Engine
{
	class TextComponent;
	class SpriteComponent;
}

class HUDItemDescription : public Component
{
public:
	COMPONENT(HUDItemDescription, "HUD Item Description");

	HUDItemDescription() = default;
	HUDItemDescription(GameObject* aGameObject);
	~HUDItemDescription();

	void Awake() override;
	void Start() override;

	void Reflect(Engine::Reflector& aReflector) override;

	void Execute(Engine::eEngineOrder aOrder) override;

	void SetName(const std::string& aName);
	void SetDesc(const std::string& aDesc);
	void SetShortDesc(const std::string& aDesc);

	void ShowShortDesc(bool aShouldShow);
	void ShowDesc(bool aShouldShow);

	void PickupItem();

private:
	float myShowDescTime = 4.0f;
	float myShowTimer = 1000.0f;

	float myFadeTime = 0.5f;
	float myFadeTimer = 0.0f;

	GameObject* myShortDescBox = nullptr;
	GameObject* myNameDescBox = nullptr;

	Engine::TextComponent* myNameText = nullptr;
	Engine::TextComponent* myDescText = nullptr;
	Engine::SpriteComponent* myBackground = nullptr;

	Engine::TextComponent* myShortNameText = nullptr;
	Engine::TextComponent* myShortDescText = nullptr;

	bool myShouldFadeOut = false;

	enum class DescState
	{
		Active,
		Fading,
		Disabled
	} myDescState = DescState::Disabled;
};
