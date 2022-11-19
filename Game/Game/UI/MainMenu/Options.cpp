#include "pch.h"
#include "Options.h"

#include "Engine/GameObject/Components/SpriteComponent.h"

void Options::Start()
{
	InitReferences();
	InitButtonCallbacks();
}

void Options::Reflect(Engine::Reflector& aReflector)
{
	aReflector.Reflect(myOptionsMap[eOptions::General].myObjectRef, "General Options");
	aReflector.Reflect(myOptionsMap[eOptions::General].myButtonRef, "General Button");

	aReflector.Reflect(myOptionsMap[eOptions::Video].myObjectRef, "Video Options");
	aReflector.Reflect(myOptionsMap[eOptions::Video].myButtonRef, "Video Button");

	aReflector.Reflect(myOptionsMap[eOptions::Audio].myObjectRef, "Audio Options");
	aReflector.Reflect(myOptionsMap[eOptions::Audio].myButtonRef, "Audio Button");
}

void Options::OnEnable()
{
	EnableOptions(myCurrentActiveOptions);
}

void Options::OnDisable()
{
	DisableAllOptions();
}

void Options::InitReferences()
{
	for (auto i = 0; i < (int)eOptions::Count; i++)
	{
		auto& context = myOptionsMap[(eOptions)i];

		if (context.myButtonRef && context.myButtonRef.Get())
		{
			context.myButtonSprite = context.myButtonRef.Get()->GetComponent<Engine::SpriteComponent>();
		}
	}
}

void Options::InitButtonCallbacks()
{
	UI::AddEvent("SaveOptions", [this](GameObject* aButtonComponent) { SaveOptions(aButtonComponent); });
	UI::AddEvent("EnableGeneral", [this](GameObject* aButtonComponent) { EnableOptions(eOptions::General); });
	UI::AddEvent("EnableVideo", [this](GameObject* aButtonComponent) { EnableOptions(eOptions::Video); });
	UI::AddEvent("EnableAudio", [this](GameObject* aButtonComponent) { EnableOptions(eOptions::Audio); });
}

void Options::EnableOptions(const eOptions& anOption)
{
	DisableAllOptions();

	auto& context = myOptionsMap[anOption];
	if (context.myObjectRef && context.myObjectRef.Get())
	{
		context.myObjectRef.Get()->SetActive(true);
	}

	if (context.myButtonSprite)
	{
		context.myButtonSprite->SetAlpha(1.0f);
	}

	myCurrentActiveOptions = anOption;
}

void Options::DisableAllOptions()
{
	for (auto i = 0; i < (int)eOptions::Count; i++)
	{
		auto& context = myOptionsMap[(eOptions)i];
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

void Options::SaveOptions(GameObject* aButtonObject)
{
	GetSettings().Save();
}

GameSettings& Options::GetSettings()
{
	static auto& settings = GameManager::GetInstance()->GetGameSettings();
	return settings;
}