#pragma once
#include "Engine/GameObject/Components/Component.h"

namespace Engine
{
	class TextComponent;
	class SpriteComponent;
	class SpriteMaterialInstance;
}

class HUDDifficulty : public Component
{
public:
	COMPONENT(HUDDifficulty, "HUD Difficulty");

	HUDDifficulty() = default;
	HUDDifficulty(GameObject* aGameObject);
	~HUDDifficulty();

	void Reflect(Engine::Reflector& aReflector) override;

	void Start();

	void Execute(Engine::eEngineOrder aOrder) override;

	void IterateStageNumber();

private:
	void UpdateTime();
	void UpdateDifficultyIcon();

	int myStageNumber = 0;

	Engine::SpriteComponent* myEye1 = nullptr;
	Engine::SpriteComponent* myEye2 = nullptr;
	Engine::SpriteComponent* myEye3 = nullptr;
	Engine::SpriteComponent* myEye4 = nullptr;
	Engine::SpriteComponent* myEye5 = nullptr;
	Engine::SpriteComponent* myEye6 = nullptr;
	Engine::SpriteComponent* myEye7 = nullptr;
	GameDifficulty myCurrentDifficulty = GameDifficulty::Beginner;

	Engine::TextComponent* myTimeText = nullptr;
	Engine::TextComponent* myTimeTextMil = nullptr;

	Engine::TextComponent* myStageText = nullptr;

	Engine::SpriteMaterialInstance* myMaterialInstance = nullptr;
};
