#pragma once

#include "MenuComponent.h"

enum class eOptions
{
	General,
	Video,
	Audio,

	Count
};

class Options : public MenuComponent
{
	COMPONENT(Options, "Options");
public:
	Options() = default;
	~Options() = default;

	void Start() override;
	void Reflect(Engine::Reflector& aReflector) override;

	void OnEnable() override;
	void OnDisable() override;

protected:
	GameSettings& GetSettings();

	void SaveOptions(GameObject* aButtonObject);

private:
	void InitReferences();
	void InitButtonCallbacks();

	void EnableOptions(const eOptions& anOption);
	void DisableAllOptions();

private:
	struct OptionContext
	{
		Engine::GameObjectRef myGeneralRef;
		Engine::GameObjectRef myObjectRef;
		Engine::GameObjectRef myButtonRef;
		Engine::SpriteComponent* myButtonSprite = nullptr;
	};

private:
	std::map<eOptions, OptionContext> myOptionsMap;
	eOptions myCurrentActiveOptions = eOptions::General;

};
