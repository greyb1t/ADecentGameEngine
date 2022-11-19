#pragma once

namespace Engine
{
	enum class WindowState;
}

enum class ResolutionOptions
{
	e1024x576,
	e1152x648,
	e1280x720,
	e1366x768,
	e1600x900,
	e1920x1080,
	e2560x1440,
	e3840x2160,

	Count
};

struct GameSettings
{
	const std::string myGameSettingsPath = "Assets/GameSettings.json";

	// Range 0.0 to 1.0, use this as a multiplier to mouse sensitivity
	float myMouseSensitivityMultiplier = 0.5f;
	float myDifficultyMultiplier = 1.0f;
	bool myHasUnlockedStarMode = false;

	float myMasterVolume = 0.6f;
	float myMusicVolume = 1.0f;
	float mySFXVolume = 1.0f;
	float myAmbienceVolume = 1.0f;

	bool myVSyncEnabled = false;
	Engine::WindowState myWindowMode = (Engine::WindowState)3;
	ResolutionOptions myResolution = ResolutionOptions::e1920x1080;

	// This just ticks true if the game has been started at least once
	bool myGameHasStartedFlag = false;

	void Save();
	void Load();
	void Apply();

	static const std::string GetWindowModeAsString(const Engine::WindowState aWindowMode);
	static const std::string GetResolutionAsString(const ResolutionOptions aResolution);
	static const Vec2ui GetResolutionAsVec2ui(const ResolutionOptions aResolution);
};

