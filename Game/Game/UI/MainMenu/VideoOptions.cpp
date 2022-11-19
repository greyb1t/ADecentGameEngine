#include "pch.h"
#include "VideoOptions.h"

#include "Engine/Renderer/GraphicsEngine.h"
#include "Engine/Renderer/WindowHandler.h"

#include "Engine/GameObject/Components/TextComponent.h"

void VideoOptions::Start()
{
	InitReferences();
	InitButtonCallbacks();
}

void VideoOptions::Reflect(Engine::Reflector& aReflector)
{
	aReflector.Reflect(myVSyncTextRef, "VSync Text");
	aReflector.Reflect(myWindowModeTextRef, "Window Mode Text");
	aReflector.Reflect(myResolutionTextRef, "Resolution Text");
}

void VideoOptions::InitReferences()
{
	if (myVSyncTextRef && myVSyncTextRef.Get())
	{
		myVSyncText = myVSyncTextRef.Get()->GetComponent<Engine::TextComponent>();
		if (myVSyncText)
		{
			myVSyncText->SetText(GetSettings().myVSyncEnabled ? "Enabled" : "Disabled");
		}
	}

	if (myWindowModeTextRef && myWindowModeTextRef.Get())
	{
		myWindowModeText = myWindowModeTextRef.Get()->GetComponent<Engine::TextComponent>();
		if (myWindowModeText)
		{
			myWindowModeText->SetText(GameSettings::GetWindowModeAsString(GetSettings().myWindowMode));
		}
	}

	if (myResolutionTextRef && myResolutionTextRef.Get())
	{
		myResolutionText = myResolutionTextRef.Get()->GetComponent<Engine::TextComponent>();
		if (myResolutionText)
		{
			myResolutionText->SetText(GameSettings::GetResolutionAsString(GetSettings().myResolution));
		}
	}
}

void VideoOptions::InitButtonCallbacks()
{
	UI::AddEvent("ApplyResolution", [this](GameObject* aButtonComponent) { Apply(aButtonComponent); });

	UI::AddEvent("ToggleVSync", [this](GameObject* aButtonComponent) { ToggleVSync(aButtonComponent); });

	UI::AddEvent("LeftArrowWindowMode", [this](GameObject* aButtonComponent) { LeftArrowWindowMode(aButtonComponent); });
	UI::AddEvent("RightArrowWindowMode", [this](GameObject* aButtonComponent) { RightArrowWindowMode(aButtonComponent); });

	UI::AddEvent("LeftArrowResolution", [this](GameObject* aButtonComponent) { LeftArrowResolution(aButtonComponent); });
	UI::AddEvent("RightArrowResolution", [this](GameObject* aButtonComponent) { RightArrowResolution(aButtonComponent); });
}

void VideoOptions::Apply(GameObject* aButtonObject)
{
	GetSettings().Apply();
	SaveOptions(aButtonObject);
}

void VideoOptions::ToggleVSync(GameObject* aButtonObject)
{
	GetSettings().myVSyncEnabled = !GetSettings().myVSyncEnabled;
	if (!myVSyncText)
	{
		LOG_ERROR(LogType::Game) << "Missing VSync text component was attached to Options";
		return;
	}
	myVSyncText->SetText(GetSettings().myVSyncEnabled ? "Enabled" : "Disabled");

	auto& window_handler = GetEngine().GetGraphicsEngine().GetWindowHandler();
	window_handler.SetVSync(GetSettings().myVSyncEnabled);
}

void VideoOptions::LeftArrowResolution(GameObject* aButtonObject)
{
	int index = (int)GetSettings().myResolution;
	index--;
	if (index < 0)
	{
		index = (int)ResolutionOptions::Count;
		index--;
	}
	GetSettings().myResolution = (ResolutionOptions)index;
	if (!myResolutionText)
	{
		LOG_ERROR(LogType::Game) << "Missing Resolution text component was attached to Options";
		return;
	}
	myResolutionText->SetText(GameSettings::GetResolutionAsString(GetSettings().myResolution));
}

void VideoOptions::RightArrowResolution(GameObject* aButtonObject)
{
	int index = (int)GetSettings().myResolution;
	index++;
	if (index >= (int)ResolutionOptions::Count)
	{
		index = 0;
	}
	GetSettings().myResolution = (ResolutionOptions)index;
	if (!myResolutionText)
	{
		LOG_ERROR(LogType::Game) << "Missing Resolution text component was attached to Options";
		return;
	}
	myResolutionText->SetText(GameSettings::GetResolutionAsString(GetSettings().myResolution));
}

void VideoOptions::LeftArrowWindowMode(GameObject* aButtonObject)
{
	int index = (int)GetSettings().myWindowMode;
	index--;
	if (index < 1)
	{
		index = (int)Engine::WindowState::Count;
		index--;
	}
	GetSettings().myWindowMode = (Engine::WindowState)index;
	if (!myWindowModeText)
	{
		LOG_ERROR(LogType::Game) << "Missing Window Mode text component was attached to Options";
		return;
	}
	myWindowModeText->SetText(GameSettings::GetWindowModeAsString(GetSettings().myWindowMode));
}

void VideoOptions::RightArrowWindowMode(GameObject* aButtonObject)
{
	int index = (int)GetSettings().myWindowMode;
	index++;
	if (index >= (int)Engine::WindowState::Count)
	{
		index = 1;
	}
	GetSettings().myWindowMode = (Engine::WindowState)index;
	if (!myWindowModeText)
	{
		LOG_ERROR(LogType::Game) << "Missing Window Mode text component was attached to Options";
		return;
	}
	myWindowModeText->SetText(GameSettings::GetWindowModeAsString(GetSettings().myWindowMode));
}