#pragma once
#include "Engine\GameObject\Components\Component.h"

namespace Engine
{
	class TextComponent;
}

class HUDInteraction;

class Interactable :
	public Component
{
	COMPONENT(Interactable, "Interactable");
public:

	Interactable() = default;
	Interactable(GameObject*);
	virtual ~Interactable();

	void Interact();
	void Hover();
	void HoverLeave();
	bool CanInteract();

	void SetCanInteract(const bool aState);

	void Awake() override;
	void Start() override;

	void Reflect(Engine::Reflector& aReflector) override;

	void ObserveInteract(const std::function<void()>& aCallback);
	void ObserveHoverEnter(const std::function<void()>& aCallback);
	void ObserveHoverExit(const std::function<void()>& aCallback);

	void SetDisplayInteractMessage(bool aDisplay);
	bool ShouldDisplayInteractMessage();

	CU::KeyCode GetKey() const;
	float GetInteractRange() const;
	void SetInteractRange(const float aRange);

protected:
	CU::KeyCode myInteractKey = CU::KeyCode::E;

	std::function<void()> myInteractObserver{};
	std::function<void()> myHoverObserver{};
	std::function<void()> myHoverLeaveObserver{};

	HUDInteraction* myHUDInteraction = nullptr;

	float myInteractableRange = 300;
	bool myCanInteract = true;
	bool myShouldDisplayInteractMessage = true;
	bool myIsHovered = false;
};