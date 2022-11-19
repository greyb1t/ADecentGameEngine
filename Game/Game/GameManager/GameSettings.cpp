#include "pch.h"
#include "GameSettings.h"

#include "Engine/Renderer/GraphicsEngine.h"
#include "Engine/Renderer/WindowHandler.h"

#include "Common/FileIO.h"
#include "Engine/Engine.h"
#include "Engine/AudioManager.h"

void GameSettings::Save()
{
	FileIO::RemoveReadOnly(myGameSettingsPath);
	std::ofstream file(myGameSettingsPath);

	nlohmann::json json;

	json["MouseSensitivity"] = myMouseSensitivityMultiplier;
	json["DifficultyMultiplier"] = myDifficultyMultiplier;
	json["StarModeUnlocked"] = myHasUnlockedStarMode;

	json["MasterVolume"] = myMasterVolume;
	json["SFXVolume"] = mySFXVolume;
	json["MusicVolume"] = myMusicVolume;
	json["AmbienceVolume"] = myAmbienceVolume;

	json["VSync"] = myVSyncEnabled;
	json["WindowMode"] = myWindowMode;
	json["Resolution"] = myResolution;

	if (file.is_open())
	{
		file << std::setw(4) << json;
		file.close();
	}
	else
	{
		LOG_ERROR(LogType::Game) << "Failed to save game settings.";
	}
}

void GameSettings::Load()
{
	if (!std::filesystem::exists(myGameSettingsPath))
		Save();

	std::ifstream file(myGameSettingsPath);

	if (!file.is_open())
	{
		LOG_ERROR(LogType::Game) << "Failed to load game settings from " << myGameSettingsPath;
		return;
	}

	nlohmann::json json = nlohmann::json::parse(file, nullptr, false, true);

	if (json.is_discarded())
	{
		LOG_ERROR(LogType::Game) << "Failed to parse game settings from " << myGameSettingsPath;
		return;
	}

	if (json.contains("MouseSensitivity"))
		myMouseSensitivityMultiplier = json["MouseSensitivity"].get<float>();

	if (json.contains("DifficultyMultiplier"))
		myDifficultyMultiplier = json["DifficultyMultiplier"].get<float>();

	if (json.contains("StarModeUnlocked"))
		myHasUnlockedStarMode = json["StarModeUnlocked"].get<bool>();

	if (json.contains("MasterVolume"))
		myMasterVolume = json["MasterVolume"].get<float>();

	if (json.contains("SFXVolume"))
		mySFXVolume = json["SFXVolume"].get<float>();

	if (json.contains("MusicVolume"))
		myMusicVolume = json["MusicVolume"].get<float>();

	if (json.contains("AmbienceVolume"))
		myAmbienceVolume = json["AmbienceVolume"].get<float>();

	if (json.contains("VSync"))
		myVSyncEnabled = json["VSync"].get<bool>();

	if (json.contains("WindowMode"))
		myWindowMode = (Engine::WindowState)json["WindowMode"].get<int>();

	if (json.contains("Resolution"))
		myResolution = (ResolutionOptions)json["Resolution"].get<int>();
}

void GameSettings::Apply()
{
	auto am = AudioManager::GetInstance();
	am->SetVolume(myMasterVolume, eVolumeType::MAIN);
	am->SetVolume(myMusicVolume, eVolumeType::MUSIC);
	am->SetVolume(mySFXVolume, eVolumeType::SFX);
	am->SetVolume(myAmbienceVolume, eVolumeType::AMB);

	auto& window_handler = GetEngine().GetGraphicsEngine().GetWindowHandler();
	window_handler.SetVSync(myVSyncEnabled);
	window_handler.SetWindowState(myWindowMode);
	window_handler.SetWindowResolution(GameSettings::GetResolutionAsVec2ui(myResolution));
}

const std::string GameSettings::GetWindowModeAsString(const Engine::WindowState aWindowMode)
{
	switch (aWindowMode)
	{
	case Engine::WindowState::Windowed: return "Windowed";
	case Engine::WindowState::WindowedBorderless: return "Windowed Borderless";
	case Engine::WindowState::Fullscreen: return "Fullscreen";
	default:
		break;
	}
	return "undefined";
}

const std::string GameSettings::GetResolutionAsString(const ResolutionOptions aResolution)
{
	switch (aResolution)
	{
	case ResolutionOptions::e1024x576: return "1024 x 576";
	case ResolutionOptions::e1152x648: return "1152 x 648";
	case ResolutionOptions::e1280x720: return "1280 x 720";
	case ResolutionOptions::e1366x768: return "1366 x 768";
	case ResolutionOptions::e1600x900: return "1600 x 900";
	case ResolutionOptions::e1920x1080: return "1920 x 1080";
	case ResolutionOptions::e2560x1440: return "2560 x 1440";
	case ResolutionOptions::e3840x2160: return "3840 x 2160";
	default:
		break;
	}
	return "undefined";
}

const Vec2ui GameSettings::GetResolutionAsVec2ui(const ResolutionOptions aResolution)
{
	switch (aResolution)
	{
	case ResolutionOptions::e1024x576: return  Vec2ui(1024, 576);
	case ResolutionOptions::e1152x648: return  Vec2ui(1152, 648);
	case ResolutionOptions::e1280x720: return  Vec2ui(1280, 720);
	case ResolutionOptions::e1366x768: return  Vec2ui(1366, 768);
	case ResolutionOptions::e1600x900: return  Vec2ui(1600, 900);
	case ResolutionOptions::e1920x1080: return Vec2ui(1920, 1080);
	case ResolutionOptions::e2560x1440: return Vec2ui(2560, 1440);
	case ResolutionOptions::e3840x2160: return Vec2ui(3840, 2160);
	default:
		break;
	}
	return Vec2ui(1920, 1080);
}
