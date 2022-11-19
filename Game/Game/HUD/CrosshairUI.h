#pragma once
#include "Engine/GameObject/Components/Component.h"
#include <Engine/GameObject/ComponentSystem/ComponentFactory.h>

namespace Engine {
	class SpriteComponent;
};

Vec2f S2W(Vec2f aScreenPos);
Vec3f S2W3(Vec2f aScreenPos);

class CrosshairUI 
	: public Component
{
public:
	COMPONENT(CrosshairUI, "Crosshair Component");                              
	CrosshairUI() = default;
	CrosshairUI(GameObject* aGameObject);
	~CrosshairUI();

	void Start() override;
	void Execute(Engine::eEngineOrder aOrder) override;
	
	void MoveLine(int aIndex, float aAngle, float aDistance);
	void SetHitMark(bool aIsActive, float aSize = 1, float aAlpha = 1);
	void SetVisible(bool anIsVisible);
private:
	bool InitLines();
	bool InitHitMark();

private:
	GameObject* myLines[4];
	GameObject* myHitMark = nullptr;
	Engine::SpriteComponent* myHitMarkSprite = nullptr;
	bool myVerified = false;
};

