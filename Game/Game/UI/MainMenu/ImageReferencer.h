#pragma once

#include "Engine/GameObject/Components/Component.h"

class ImageReferencer : public Component
{
	COMPONENT(ImageReferencer, "ImageReferencer");
public:
	ImageReferencer() = default;
	~ImageReferencer() = default;

	void Start() override;
	void Reflect(Engine::Reflector& aReflector) override;

	void OnEnable() override;
	void OnDisable() override;

	void Execute(Engine::eEngineOrder aOrder) override;

	void InstantDisable();

	void EnableImage();
	void DisableImage();

private:
	Engine::GameObjectRef myImageRef;
	Engine::SpriteComponent* mySprite = nullptr;

	bool myWillFade = false;
	float myStartSize = 0.8f;
	float myEndSize = 1.0f;

	float myStartScale = 0.8f;
	float myGoalScale = 1.0f;

	float myStartAlpha = 0.0f;
	float myGoalAlpha = 1.0f;

	float myFadeProgress = 0.0f;
	float myFadeDuration = 1.0f;
};