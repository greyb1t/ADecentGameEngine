#pragma once
#include "Engine/GameObject/Components/Component.h"

namespace Engine
{
	class TextComponent;
	class ButtonComponent;
	class SpriteComponent;
}

class CameraController;
class InventoryComponent;
class HUDItemDescription;
class Player;

struct ItemHover
{
	float myHoverFadeTime = 0.2f;
	float myHoverFadeTimer = 0.0f;
	bool myIsShowingHover = false;

	Engine::SpriteComponent* mySprite = nullptr;

	void UpdateHover();
};

class PauseMenuComponent : public Component
{
public:
	COMPONENT(PauseMenuComponent, "Pause Menu Component");

	PauseMenuComponent() = default;
	PauseMenuComponent(GameObject* aGameObject);
	~PauseMenuComponent();

	void Reflect(Engine::Reflector& aReflector) override;

	void Start() override;

	void Execute(Engine::eEngineOrder aOrder) override;

	void SetPauseListener(std::function<void(bool)> aListener);
	void SetInventory(InventoryComponent* aInventory);

private:
	void TogglePause();

	void UpdatePlayerStats();
	const std::string GetGoodFloatText(float aFloat);

	void Resume(GameObject* aButton);
	void Settings(GameObject* aButton);
	void Quit(GameObject* aButton);

	void SureYes();
	void SureNo();

	void InitAudio();

	Engine::SpriteComponent* myBackground = nullptr;
	Engine::SpriteComponent* myItemBox = nullptr;

	CameraController* myCamera = nullptr;
	InventoryComponent* myInventory = nullptr;

	Player* myPlayer = nullptr;

	HUDItemDescription* myItemDescription = nullptr;

	ItemHover myFirstItemHover;
	ItemHover mySecondItemHover;
	ItemHover* myCurrentItemHover = nullptr;
	Item* myLastItem = nullptr;

	bool myIsPaused = false;
	bool myIsShowingSettings = false;
	bool myIsShowingAreYouSure = false;

	std::function<void(bool)> myPauseListener;

	Engine::TextComponent* myStatsText;

	GameObject* myResumeButton = nullptr;
	GameObject* mySettingsButton = nullptr;
	GameObject* myQuitButton = nullptr;

	GameObject* myAreYouSure = nullptr;

	GameObjectPrefabRef mySettingsRef;
	GameObject* mySettings = nullptr;
	Engine::AudioComponent* myAudioComponent = nullptr;
};
