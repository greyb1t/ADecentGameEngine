#pragma once
namespace Engine
{
	class TextComponent;
}

class Interactable;
class InteractableManager
{
public:
	void Register(Interactable*), Unregister(Interactable*);

	Interactable* SearchInteractable(Vec3f aPosition, Vec3f aLookDirection);

	static InteractableManager& Instance();

private:
	std::vector<Interactable*> myInteractables;
	Weak<GameObject> myTextObject;
	Interactable* myLastFoundInteractable = nullptr;
};