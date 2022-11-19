#pragma once
#include "Engine\GameObject\Components\Component.h"

namespace Engine
{
	class TextComponent;
}

enum class eDamageType;

class DamageNumber :
    public Component
{
public:
    COMPONENT(DamageNumber, "Damage Number");

    DamageNumber() = default;
    DamageNumber(GameObject*, int aNumber);
    void Start() override;

    void SetColor(const CU::Vector4f& aColor);
    void SetTextScale(const float aScale);
    void SetPreset(eDamageType aPreset);


    void Execute(Engine::eEngineOrder aOrder) override;
private:
    Vec4f myTextColor = { 1.f, .2f, .2f, 1.f };
    Vec3f myOriginalScale = {0.f,0.f,0.f};
    Vec2f myShakeOffset = { 0.f, 0.f };
    Engine::TextComponent* myText = nullptr;
    int myNumber = 0;
    float myShakeInterval = -1.f;
    float myShakeTimer = 0.f;
    float myShakeLengthReduction = 0.7f;
    float myLifetime = 1.f;
    float myTime = 0;
    float myRiseSpeed = 500.f;
    float myScaleModifier = 1.f;
};

