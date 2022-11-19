#pragma once

#include "Options.h"

class VideoOptions : public Options
{
	COMPONENT(VideoOptions, "VideoOptions");
public:
	VideoOptions() = default;
	~VideoOptions() = default;

	void Start() override;
	void Reflect(Engine::Reflector& aReflector) override;

private:
	void InitReferences();
	void InitButtonCallbacks();

	void Apply(GameObject* aButtonObject);

	void ToggleVSync(GameObject* aButtonObject);

	void LeftArrowWindowMode(GameObject* aButtonObject);
	void RightArrowWindowMode(GameObject* aButtonObject);

	void LeftArrowResolution(GameObject* aButtonObject);
	void RightArrowResolution(GameObject* aButtonObject);

private:
	Engine::GameObjectRef myVSyncTextRef;
	Engine::TextComponent* myVSyncText = nullptr;

	Engine::GameObjectRef myWindowModeTextRef;
	Engine::TextComponent* myWindowModeText = nullptr;

	Engine::GameObjectRef myResolutionTextRef;
	Engine::TextComponent* myResolutionText = nullptr;

};

