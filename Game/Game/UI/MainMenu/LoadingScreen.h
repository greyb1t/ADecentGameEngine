#pragma once

#include "MenuComponent.h"

class LoadingScreen : public MenuComponent
{
	COMPONENT(LoadingScreen, "Loading Screen");
public:
	LoadingScreen() = default;
	~LoadingScreen() = default;

	void Start() override;
	void OnEnable() override;
	void Reflect(Engine::Reflector& aReflector) override;

private:
	Engine::GameObjectRef myImage;
	Engine::GameObjectRef myText;

	bool myBeginEnabled = true;
	std::vector<TextureRef> myRandomImages;
	std::vector<std::string> myRandomTips;

};