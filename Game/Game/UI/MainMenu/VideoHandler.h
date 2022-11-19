#pragma once

#include "Engine/GameObject/Components/Component.h"
#include "Engine/GameObject/GameObject.h"

class VideoHandler : public Component
{
	COMPONENT(VideoHandler, "Video Handler");
public:
	VideoHandler() = default;
	~VideoHandler() = default;

	void Awake() override;

	void Start() override;
	void Execute(Engine::eEngineOrder aOrder) override;

	void Reflect(Engine::Reflector& aReflector) override;
	void StartGame(const std::string& aPath = "");
	
	void DisableAll();

	void SetLevelIsLoading();

private:
	void EnableMainMenu();

	void LoadLevel(const std::string& aPath);

private:
	Engine::GameObjectRef mySplashVideo;
	Engine::GameObjectRef myBackgroundVideo;
	Engine::GameObjectRef myStartGameVideo;
	Engine::GameObjectRef myMenuHandler;

	bool myStartGameVideoPlaying = false;
	bool myLevelIsLoading = false;
	bool mySplashSkipped = false;

	float myOldFuck = 0.0f;
	bool myShitFuck = false;
};