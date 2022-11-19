#pragma once
#include "Engine/GameObject/Components/Component.h"

namespace Engine
{
	class TextComponent;
	class SpriteComponent;
}

class HUDAbility : public Component
{
public:
	COMPONENT(HUDAbility, "HUD Ability Component");

	HUDAbility() = default;
	HUDAbility(GameObject* aGameObject);
	~HUDAbility();

	void Reflect(Engine::Reflector& aReflector) override;

	void Start() override;

	void Execute(Engine::eEngineOrder aOrder) override;

	void StartCooldown();
	void ResetCooldown();

	void SetCooldownTime(const float& aTime);

private:
	float myCooldownTime = 5.0f;
	float myTimer = 0.0f;

	float myEffectTime = 0.1f;
	float myEffectTimer = 0.0f;

	Engine::SpriteComponent* myAbilityIcon = nullptr;
	Engine::SpriteComponent* myCooldownOverlay = nullptr;
	Engine::SpriteComponent* myEffect = nullptr;

	Engine::TextComponent* myTimerText = nullptr;
};
