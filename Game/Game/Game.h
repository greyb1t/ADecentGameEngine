#pragma once

namespace Engine
{
	class LogoScene;
	class Engine;
	class ResourceReferences;
}

class Game
{
public:
	Game();
	~Game();

	bool Init();

	void Update();
private:
	void RegisterSingletonComponents();
	void RegisterComponents();
	void RegisterPhysicsMaterials();
	void RegisterScenes();
private:
	Owned<Engine::Engine> myEngine = nullptr;

	Shared<Engine::LogoScene> myLogoScene;
};