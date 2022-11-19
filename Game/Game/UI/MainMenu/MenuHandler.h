#pragma once

#include "Engine/GameObject/Components/Component.h"

class MenuHandler : public Component
{
	COMPONENT(MenuHandler, "Menu Handler");
public:
	MenuHandler() = default;
	~MenuHandler() = default;

	void Awake() override;
	void Start() override;

	void Reflect(Engine::Reflector& aReflector) override;
	void Execute(Engine::eEngineOrder aOrder) override;

	void AnimateIntro();

	void EnableMainMenu();
	void EnablePlay();
	void EnableLevelSelect();
	void EnableHowToPlay();
	void EnableOptions();
	void EnableCredits();
	void EnableLoadingScreen();

	void SkipToVideoLoop();
	void SkipMenuAnimation();
	void DisableAll();

	void SetSelectedLevel(const std::string& aLevelPath);
	void ClearSelectedLevel();

	const std::string& GetSelectedLevel() const;

private:
	void NotStarted();
	void Animating();
	void Finished();

private:
	Engine::GameObjectRef myTitleImageRef;
	Engine::GameObjectRef myBackgroundVideoRef;
	Engine::GameObjectRef myMainMenuRef;
	Engine::GameObjectRef myPlayRef;
	Engine::GameObjectRef myLevelSelectRef;
	Engine::GameObjectRef myHowToPlayRef;
	Engine::GameObjectRef myOptionsRef;
	Engine::GameObjectRef myCreditsRef;
	Engine::GameObjectRef myLoadingScreenRef;

	Engine::SpriteComponent* myTitleImage = nullptr;
	Engine::VideoComponent* myBackgroundVideo = nullptr;

	std::string mySelectedLevel = "";

	enum class MenuState
	{
		NotStarted,
		Animating,
		Finished
	} myMenuState = MenuState::NotStarted;

	struct MenuAnimation
	{
		GameObject* myMenuObj = nullptr;
		Vec3f myStartPos = {};
		Vec3f myEndPos = {};

		float myStartTime = 0.0f;

		float myProgress = 0.0f;
		float myDuration = 1.0f;
	};

	std::vector<MenuAnimation> myAnims;
	float myAnimationProgress = 0.0f;
	float myAnimationDuration = 1.5f;

	float myTitleImageFadeProgress = 0.0f;
	float myTitleImageFadeDuration = 1.0f;

	float myAnimationStartDelayProgress = 0.0f;
	float myAnimationStartDelayDuration = 3.0f;
};

