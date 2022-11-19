#pragma once

#include "ResourceManagement/ResourceManagerSettings.h"
#include "Renderer/WindowHandler.h"

struct EngineSettings
{
	std::string WindowTitle;
	Vec2ui WindowSize;
	Vec2ui myTargetWindowSize;
	Engine::WindowState myWindowState = Engine::WindowState::Windowed;
	HCURSOR myCursorHandle = nullptr;
	bool myEnableFilewatcher = false;

	Engine::ResourceManagerSettings myResourceManagerSettings;
};