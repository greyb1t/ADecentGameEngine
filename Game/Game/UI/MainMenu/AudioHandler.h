#pragma once

#include "MenuComponent.h"

class AudioHandler : public MenuComponent
{
	COMPONENT(AudioHandler, "AudioHandler");
public:
	AudioHandler() = default;
	~AudioHandler() = default;

	void Start() override;

	void Reflect(Engine::Reflector& aReflector) override;

private:
	void DisableMainMenuSnapshot();
	void PlayMainMenuEventSound();

	void PlayHoverSound(); 
	void PlayClickSound();

	Engine::AudioComponent* myAudioComponent = nullptr;

	Engine::GameObjectRef myLevel1Button;
	Engine::GameObjectRef myLevel2Button;
	Engine::GameObjectRef myLevel3Button;
	Engine::GameObjectRef myLevel4Button;
	Engine::GameObjectRef myLevel5Button;
	Engine::GameObjectRef myStartGameButton;
};
