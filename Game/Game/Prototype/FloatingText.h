#pragma once
#include "Engine\GameObject\Components\Component.h"
#include <string>

namespace Engine
{
	class TextComponent;
}

class FloatingText :
    public Component
{
public:
    COMPONENT(FloatingText, "Floating Text");

    FloatingText() = default;
    FloatingText(GameObject*, const std::string aText);
    void Start() override;

    void Execute(Engine::eEngineOrder aOrder) override;
private:
    Engine::TextComponent* myTextComp = nullptr;
    std::string myText = "";
    float myLifetime = 1.f;
    float myTime = 0;
    float myRiseSpeed = 500.f;
    Vec4f myTextColor = { 1.f, .2f, .2f, 1.f };
};

