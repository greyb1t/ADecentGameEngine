#include "pch.h"
#include "HowToPlay.h"

#include "Engine/GameObject/Components/SpriteComponent.h"

void HowToPlay::Start()
{
	InitReferences();
	InitButtonCallbacks();
}

void HowToPlay::Reflect(Engine::Reflector& aReflector)
{
	aReflector.Reflect(myHowToPlayMap[eHowToPlay::Controls].myObjectRef, "Controls Options");
	aReflector.Reflect(myHowToPlayMap[eHowToPlay::Controls].myButtonRef, "Controls Button");

	aReflector.Reflect(myHowToPlayMap[eHowToPlay::Abilities].myObjectRef, "Abilities Options");
	aReflector.Reflect(myHowToPlayMap[eHowToPlay::Abilities].myButtonRef, "Abilities Button");

	aReflector.Reflect(myHowToPlayMap[eHowToPlay::Items].myObjectRef, "Items Options");
	aReflector.Reflect(myHowToPlayMap[eHowToPlay::Items].myButtonRef, "Items Button");
}

void HowToPlay::OnEnable()
{
	EnableHowToPlay(myCurrentActiveHowToPlay);
}

void HowToPlay::OnDisable()
{
	DisableAllHowToPlay();
}

void HowToPlay::InitReferences()
{
	for (auto i = 0; i < (int)eHowToPlay::Count; i++)
	{
		auto& context = myHowToPlayMap[(eHowToPlay)i];

		if (context.myButtonRef && context.myButtonRef.Get())
		{
			context.myButtonSprite = context.myButtonRef.Get()->GetComponent<Engine::SpriteComponent>();
		}
	}
}

void HowToPlay::InitButtonCallbacks()
{
	UI::AddEvent("EnableControls", [this](GameObject* aButtonComponent) { EnableHowToPlay(eHowToPlay::Controls); });
	UI::AddEvent("EnableAbilities", [this](GameObject* aButtonComponent) { EnableHowToPlay(eHowToPlay::Abilities); });
	UI::AddEvent("EnableItems", [this](GameObject* aButtonComponent) { EnableHowToPlay(eHowToPlay::Items); });
}

void HowToPlay::EnableHowToPlay(const eHowToPlay& anOption)
{
	DisableAllHowToPlay();

	auto& context = myHowToPlayMap[anOption];
	if (context.myObjectRef && context.myObjectRef.Get())
	{
		context.myObjectRef.Get()->SetActive(true);
	}

	if (context.myButtonSprite)
	{
		context.myButtonSprite->SetAlpha(1.0f);
	}

	myCurrentActiveHowToPlay = anOption;
}

void HowToPlay::DisableAllHowToPlay()
{
	for (auto i = 0; i < (int)eHowToPlay::Count; i++)
	{
		auto& context = myHowToPlayMap[(eHowToPlay)i];
		if (context.myObjectRef && context.myObjectRef.Get())
		{
			context.myObjectRef.Get()->SetActive(false);
		}

		if (context.myButtonSprite)
		{
			context.myButtonSprite->SetAlpha(0.25f);
		}
	}
}
