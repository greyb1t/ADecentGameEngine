#include "pch.h"
#include "Interactable.h"

#include "InteractableManager.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/GameObject/Components/TextComponent.h"
#include "Engine/Reflection/Reflector.h"

#include "HUD/HUDHandler.h"
#include "HUD/HUDInteraction.h"

//Engine::TextComponent* Interactable::ourText = nullptr;

Interactable::Interactable(GameObject* aGameObject)
	: Component(aGameObject)
{}

Interactable::~Interactable()
{
	InteractableManager::Instance().Unregister(this);
}

void Interactable::Interact()
{
	if (myInteractObserver && myCanInteract)
	{
		myInteractObserver();
	}
}

void Interactable::Hover()
{
	if (myHUDInteraction)
	{
		myHUDInteraction->ShowDesc(true);
		myHUDInteraction->SetText("");
	}

	if(myHoverObserver && myCanInteract)
	{
		myHoverObserver();
	}

	myIsHovered = true;
}

void Interactable::HoverLeave()
{
	if (myHUDInteraction)
	{
		myHUDInteraction->ShowDesc(false);
	}

	if(myHoverLeaveObserver)
	{
		myHoverLeaveObserver();
	}

	myIsHovered = false;
}

bool Interactable::CanInteract()
{
	return myCanInteract;
}

void Interactable::SetCanInteract(const bool aState)
{
	myCanInteract = aState;
}

void Interactable::Awake()
{
	InteractableManager::Instance().Register(this);
}

void Interactable::Start()
{
	auto player = Main::GetPlayer();
	if (player)
	{
		myHUDInteraction = player->GetComponent<HUDHandler>()->GetInteractionBox();
	}
}

void Interactable::Reflect(Engine::Reflector& aReflector)
{
	aReflector.Reflect(myInteractableRange, "Interaction Range");
	aReflector.Reflect(myCanInteract, "Is Interactable");
}

void Interactable::ObserveInteract(const std::function<void()>& aCallback)
{
	myInteractObserver = aCallback;
}

void Interactable::ObserveHoverEnter(const std::function<void()>& aCallback)
{
	myHoverObserver = aCallback;
}

void Interactable::ObserveHoverExit(const std::function<void()>& aCallback)
{
	myHoverLeaveObserver = aCallback;
}

void Interactable::SetDisplayInteractMessage(bool aDisplay)
{
	myShouldDisplayInteractMessage = aDisplay;
}

bool Interactable::ShouldDisplayInteractMessage()
{
	return myShouldDisplayInteractMessage;
}

CU::KeyCode Interactable::GetKey() const
{
	return myInteractKey;
}

float Interactable::GetInteractRange() const
{
	return myInteractableRange;
}

void Interactable::SetInteractRange(const float aRange)
{
	myInteractableRange = aRange;
}
