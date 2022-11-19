#pragma once

#include "MenuComponent.h"

namespace Engine
{
	class SliderComponent;
}

class Leaderboard;

struct DifficultySettings : public Engine::Reflectable
{
	DifficultySettings() = default;
	DifficultySettings(std::string a, float b) : myLabel(a), myMultiplier(b) { };

	std::string myLabel = "";
	float myMultiplier = 1.0f;
	TextureRef mySprite;

	void Reflect(Engine::Reflector& aReflector) override;
};

class Play : public MenuComponent
{
	COMPONENT(Play, "Play");
public:
	Play() = default;
	~Play() = default;

	void Start() override;
	void Reflect(Engine::Reflector& aReflector) override;

	void OnEnable() override;
	void OnDisable() override;

private:
	void InitButtonCallbacks();
	void InitSliderStartValues();

	void Difficulty(const float aSliderValue);

	void SetDifficulty(const float aMultiplierValue);

	void SaveDifficulty();

	void StartGame();

private:
	TextureRef myStarModeSpriteRef;

	Engine::SliderComponent* myDifficultySlider = nullptr;
	Engine::TextComponent* myDifficultyText = nullptr;
	Engine::SpriteComponent* myDifficultySprite = nullptr;
	Engine::AudioComponent* myAudioComponent = nullptr;

	Engine::GameObjectRef myVideoHandlerRef;
	Engine::GameObjectRef myDifficultyRef;
	Engine::GameObjectRef myDifficultyTextRef;
	Engine::GameObjectRef myDifficultySpriteRef;
	Engine::GameObjectRef myLeaderboardRef;

	Leaderboard* myLeaderboard = nullptr;

	std::vector<DifficultySettings> myDifficulties = {
		{ "Easy" , 0.5f },
		{ "Normal", 1.0f },
		{ "Hard", 1.5f}
	};
	DifficultySettings myCurrentDifficulty;

	DifficultySettings myStarModeDifficulty =
	{
		"Star Mode", 8.0f
	};

};