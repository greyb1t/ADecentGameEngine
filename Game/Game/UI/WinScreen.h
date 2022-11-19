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

class WinScreen : public Component
{
public:
	COMPONENT(WinScreen, "Win Screen Component");

	WinScreen() = default;
	WinScreen(GameObject* aGameObject);
	~WinScreen();

	void Reflect(Engine::Reflector& aReflector) override;

	void Start() override;

	void Execute(Engine::eEngineOrder aOrder) override;

private:
	void UpdatePlayerStats();
	const std::string GetGoodFloatText(float aFloat);

	void Quit(GameObject* aButton);

	void InitAudio();

	void SetTimeText();

	void SetDifficulty();

	void SaveToLeaderboard();

	bool myIsFirstFrame = true;
	float myLastTime = 0.0f;
	
	CameraController* myCamera = nullptr;
	InventoryComponent* myInventory = nullptr;

	Player* myPlayer = nullptr;

	HUDItemDescription* myItemDescription = nullptr;

	Engine::TextComponent* myStatsText;

	Engine::AudioComponent* myAudioComponent = nullptr;

	Engine::TextComponent* myTimeText = nullptr;
	Engine::TextComponent* myTimeTextMil = nullptr;

	Engine::SpriteComponent* myEye1 = nullptr;
	Engine::SpriteComponent* myEye2 = nullptr;
	Engine::SpriteComponent* myEye3 = nullptr;
	Engine::SpriteComponent* myEye4 = nullptr;
	Engine::SpriteComponent* myEye5 = nullptr;
	Engine::SpriteComponent* myEye6 = nullptr;
	Engine::SpriteComponent* myEye7 = nullptr;
	Engine::TextComponent* myDifficultyText = nullptr;

	Engine::SpriteComponent* myStarPopup = nullptr;
};
