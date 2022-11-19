#include "pch.h"
#include "LoadingScreen.h"

#include "Engine/GameObject/Components/SpriteComponent.h"
#include "Engine/GameObject/Components/TextComponent.h"
#include "Engine/AudioManager.h"

void LoadingScreen::Start()
{
	myGameObject->SetActive(myBeginEnabled);
}

void LoadingScreen::OnEnable()
{
	AudioManager::GetInstance()->StopAll();

	if (myImage && myImage.Get())
	{
		if (auto sprite = myImage.Get()->GetComponent<Engine::SpriteComponent>())
		{
			if (!myRandomImages.empty())
			{
				const int rnd = Random::RandomInt(0, myRandomImages.size() - 1);
				auto& chosen = myRandomImages[rnd];
				sprite->SetSprite(chosen);
			}
		}
	}

	if (myText && myText.Get())
	{
		if (auto text = myText.Get()->GetComponent<Engine::TextComponent>())
		{
			if (!myRandomTips.empty())
			{
				const int rnd = Random::RandomInt(0, myRandomTips.size() - 1);
				auto& chosen = myRandomTips[rnd];
				text->SetText(chosen);
			}
		}
	}
}

void LoadingScreen::Reflect(Engine::Reflector& aReflector)
{
	aReflector.Reflect(myBeginEnabled, "Begin Enabled?");

	aReflector.Reflect(myImage, "Image Sprite");
	aReflector.Reflect(myText, "Tips Text");

	aReflector.Reflect(myRandomImages, "Images");
	aReflector.ReflectGroup([&aReflector, this]()
		{
			aReflector.Reflect(myRandomTips, "Tips");
		}, "Tips");

}
