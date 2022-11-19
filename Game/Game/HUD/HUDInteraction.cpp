#include "pch.h"
#include "HUDInteraction.h"

#include "Engine/Engine.h"

#include "Engine/GameObject/Components/SpriteComponent.h"
#include "Engine/GameObject/Components/TextComponent.h"

#include "Engine\Reflection\Reflector.h"
#include "Engine\GameObject\GameObject.h"

HUDInteraction::HUDInteraction(GameObject* aGameObject) : Component(aGameObject)
{
}

HUDInteraction::~HUDInteraction()
{

}

void HUDInteraction::Reflect(Engine::Reflector& aReflector)
{
	aReflector.Reflect(myInteractionText, "InteractionText");
}

void HUDInteraction::Start()
{
	myNameText = GetTransform().GetChildByGameObjectName("Text")->GetComponent<Engine::TextComponent>();
	myBackground = GetTransform().GetChildByGameObjectName("Background")->GetComponent<Engine::SpriteComponent>();

	myDeclineTimer = myDeclineTime;
}

void HUDInteraction::Execute(Engine::eEngineOrder aOrder)
{
	myDeclineTimer += Time::DeltaTime;
	myDeclineTimer = std::clamp(myDeclineTimer, 0.0f, myDeclineTime);

	float percent = myDeclineTimer / myDeclineTime;

	myNameText->SetColor(Vec4f(1.0f, percent, percent, 1.0f));
}

void HUDInteraction::SetInteractionName(const std::string& aName)
{
	SetText(myInteractionText + aName);
}

void HUDInteraction::SetText(const std::string& aText)
{
	if (aText == "")
	{
		myNameText->SetText("Press E to Interact");
	}
	else
	{
		myNameText->SetText(aText);
	}
}

void HUDInteraction::ShowDesc(bool aShouldShow)
{
	myGameObject->SetActive(aShouldShow);
}

void HUDInteraction::Decline()
{
	myDeclineTimer = 0.0f;
}
