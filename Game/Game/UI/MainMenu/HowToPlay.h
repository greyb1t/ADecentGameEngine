#pragma once

#include "MenuComponent.h"

enum class eHowToPlay
{
	Controls,
	Abilities,
	Items,

	Count
};

class HowToPlay : public MenuComponent
{
	COMPONENT(HowToPlay, "HowToPlay");
public:
	HowToPlay() = default;
	~HowToPlay() = default;

	void Start() override;
	void Reflect(Engine::Reflector& aReflector) override;

	void OnEnable() override;
	void OnDisable() override;

private:
	void InitReferences();
	void InitButtonCallbacks();

	void EnableHowToPlay(const eHowToPlay& anOption);
	void DisableAllHowToPlay();

private:
	struct OptionContext
	{
		Engine::GameObjectRef myGeneralRef;
		Engine::GameObjectRef myObjectRef;
		Engine::GameObjectRef myButtonRef;
		Engine::SpriteComponent* myButtonSprite = nullptr;
	};

private:
	std::map<eHowToPlay, OptionContext> myHowToPlayMap;
	eHowToPlay myCurrentActiveHowToPlay = eHowToPlay::Controls;

};