#pragma once
#include "Engine/GameObject/Components/Component.h"

#include "Engine/GameObject/Components/SpriteComponent.h"

namespace Engine
{
	class TextComponent;
}

class HUDAbility;
class HUDHealth;
class HUDItemDescription;
class HUDInteraction;
class HUDDifficulty;
class CrosshairUI;

class HUDHandler : public Component
{
public:
	enum class AbilityType
	{
		PrimaryFire,
		SecondaryFire,
		Mobility,
		Utiliy
	};

	COMPONENT(HUDHandler, "HUD Handler Component");

	HUDHandler() = default;
	HUDHandler(GameObject* aGameObject);
	~HUDHandler();

	void Reflect(Engine::Reflector& aReflector) override;

	void Awake() override;

	void Start() override;

	void Execute(Engine::eEngineOrder aOrder) override;

	void StartCooldown(AbilityType aAbility);
	void ResetAllCooldowns();

	HUDAbility* GetAbility(AbilityType aAbility);

	HUDHealth* GetHealth();

	HUDItemDescription* GetItemDescriptionBox();
	HUDInteraction* GetInteractionBox();
	CrosshairUI* GetCrosshairUI();

	void SetMoneyAmmount(int aMoney);

	void IterateStageNumber();

	void ActivateDeathScreen();

	void SetObjective(const std::string& aObjective);

	void FadeOut(const std::function<void()>& aCallback, bool aShouldFadeBackIn = false);

	void SetCrosshairVisible(bool anIsVisible);

private:
	bool SetAbility(Transform& aToSearchFrom, const std::string& aGameobjectName, HUDAbility*& aObjectToAssign);

	void ErrorAndDisable();

	void DeathRetry(GameObject* aButton);
	void DeathMainMenu(GameObject* aButton);

	void ActuallyActivateDeathScreen();

	GameObject* myDeathScreen = nullptr;

	HUDAbility* myPrimaryFire = nullptr;
	HUDAbility* mySecondaryFire = nullptr;
	HUDAbility* myMobility = nullptr;
	HUDAbility* myUtiliy = nullptr;

	HUDHealth* myHealth = nullptr;

	HUDItemDescription* myItemDescription = nullptr;

	HUDInteraction* myInteraction = nullptr;
	CrosshairUI* myCrosshair = nullptr;

	HUDDifficulty* myHUDDifficulty = nullptr;

	Engine::TextComponent* myMoneyAmmount = nullptr;

	Engine::TextComponent* myObjective = nullptr;

	bool myHasDied = false;
	float myDeathDelay = 3.0f;
	float myDeathTimer = 0.0f;
	bool myDeathScreenIsOpen = false;

	Engine::SpriteComponent* myFade = nullptr;
	std::function<void()> myFadeOutCallback = nullptr;

	bool myHasDoneCallback = false;
	bool myIsBlack = false;
	float myFadeTime = 0.5f;
	float myFadeTimer = 0.0f;

	float mySpawnBlackTime = 1.0f;
	float mySpawnBlackTimer = 0.0f;

	bool myShouldFadeBackIn = false;
};
