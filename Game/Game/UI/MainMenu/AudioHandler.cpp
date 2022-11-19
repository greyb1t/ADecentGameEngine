#include "pch.h"
#include "AudioHandler.h"
#include "Engine/GameObject/Components/MusicManager.h"
#include "Engine/GameObject/Components/AudioComponent.h"

void AudioHandler::Start()
{
	UI::AddEvent("LeaveMenu", [this](GameObject* aButtonObject) { DisableMainMenuSnapshot(); });
	UI::AddEvent("HoverAudio", [this](GameObject* aButtonObject) { PlayHoverSound(); });
	UI::AddEvent("ClickAudio", [this](GameObject* aButtonObject) { PlayClickSound(); });

	auto* musicManager = myGameObject->GetSingletonComponent<Engine::MusicManager>();
	myAudioComponent = musicManager->GetGameObject()->GetComponent<Engine::AudioComponent>();

	myAudioComponent->AddEvent("MenuSnapshot", "snapshot:/GAMESTATE/Menu");
	myAudioComponent->AddEvent("Hover", "event:/UI/MouseHovver");
	myAudioComponent->AddEvent("Click", "event:/UI/ButtonClick");

	myAudioComponent->PlayEvent("MenuSnapshot");
}

void AudioHandler::Reflect(Engine::Reflector& aReflector)
{
	/*aReflector.Reflect(myStartGameButton, "Start Game Button");
	aReflector.Reflect(myLevel1Button, "Level 1 Button");
	aReflector.Reflect(myLevel2Button, "Level 2 Button");
	aReflector.Reflect(myLevel3Button, "Level 3 Button");
	aReflector.Reflect(myLevel4Button, "Level 4 Button");
	aReflector.Reflect(myLevel5Button, "Level 5 Button");*/
}
void AudioHandler::PlayHoverSound()
{
	myAudioComponent->PlayEvent("Hover");
}
void AudioHandler::PlayClickSound()
{
	myAudioComponent->PlayEvent("Click");
}
void AudioHandler::DisableMainMenuSnapshot()
{
	myAudioComponent->StopEvent("MenuSnapshot");
}

void AudioHandler::PlayMainMenuEventSound()
{

}

