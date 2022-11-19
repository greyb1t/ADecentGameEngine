#pragma once
#include "Engine/GameObject/Components/Component.h"

namespace Engine
{
	class SpriteComponent;
	class TextComponent;
}

class HUDHealth : public Component
{
public:
	COMPONENT(HUDHealth, "HUD Health Component");

	HUDHealth() = default;
	HUDHealth(GameObject* aGameObject);
	~HUDHealth();

	void Start() override;
	void Execute(Engine::eEngineOrder) override;

	void SetMaxHealth(const float& aMaxHealth);
	void SetHealth(const float& aHealth);

	void SetMaxShield(const float& aMaxShield);
	void SetShield(const float& aShield);
	void SetShields(const float& aShield, const float& aMaxShield);

	void SetBarrier(const float& aBarrier);

private:
	void CalculateHealth();

	float myMaxHealth = 0.0f;
	float myHealth = 0.0f;

	float myMaxShield = 0.0f;
	float myShield = 0.0f;

	float myBarrier = 0.0f;

	float myHealthPercent = 1.f;
	float myHealthPercentTarget = 1.f;
	bool myIsIncreasing = false;

	float myFlashTimer = .1f;
	bool myFlashSwitch = false;
	CU::Vector4f myHealthSpriteColor{};
	CU::Vector4f myShieldSpriteColor{};
	CU::Vector4f myBarrierSpriteColor{};
	CU::Vector4f myShieldBackgroundColor{};

	Engine::TextComponent* myHealthText = nullptr;

	Engine::SpriteComponent* myHealthSprite = nullptr;
	Engine::SpriteComponent* myHealthDiffSprite = nullptr;
	Engine::SpriteComponent* myShieldSprite = nullptr;
	Engine::SpriteComponent* myShieldBackground = nullptr;
	Engine::SpriteComponent* myBarrierSprite = nullptr;

	static constexpr float ourDangerHealthPercentage = 0.25f;
};
