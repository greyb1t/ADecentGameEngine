#include "pch.h"
#include "InteractableManager.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/GameObject/Components/TextComponent.h"
#include "Interactable.h"

void InteractableManager::Register(Interactable* aInteractable)
{
	myInteractables.emplace_back(aInteractable);
	if (myInteractables.size() == 1)
	{
		GameObject* object = aInteractable->GetGameObject()->GetScene()->AddGameObject<GameObject>();
		object->GetTransform().SetPosition({ 0.5f, -0.5f , 0.f});
		myTextObject = object->GetWeak();
		if (auto shared = myTextObject.lock())
		{
			shared->GetTransform().SetScale(.6f);
			Engine::TextComponent* text = shared->AddComponent<Engine::TextComponent>("Press E to interact...", 0);
			text->SetIsInScreenSpace(true);
			text->SetActive(false);
		}
	}
}

void InteractableManager::Unregister(Interactable* aInteractable)
{
	auto it = std::find(myInteractables.begin(), myInteractables.end(), aInteractable);
	if (it != myInteractables.end())
	{
		if(myLastFoundInteractable == *it)
		{
			myLastFoundInteractable = nullptr;
		}

		myInteractables.erase(it);
	}
	if (!myInteractables.size())
	{
		if (auto shared = myTextObject.lock())
		{
			shared->Destroy(); 
		}
	}
}

Interactable* InteractableManager::SearchInteractable(Vec3f aPosition, Vec3f aLookDirection)
{
	Interactable* choice = nullptr;
	float lastDot = 0.f;

	for (int i = 0; i < myInteractables.size(); i++)
	{
		auto& interactable = myInteractables[i];

		if(!interactable->CanInteract())
		{
			continue;
		}

		auto deltaPos = interactable->GetTransform().GetPosition() - aPosition;

		float distance = deltaPos.Length();
		deltaPos.y = 0.f;
		deltaPos.Normalize();
		aLookDirection.y = 0.f;
		aLookDirection.Normalize();
 		const float dot = aLookDirection.Dot(deltaPos);

		float interactRange = interactable->GetInteractRange();
		if(interactable == myLastFoundInteractable)
		{
			interactRange += 50.f;
		}

		if (distance < interactRange && dot > 0)
		{
			if(dot > lastDot)
			{
				choice = interactable;
				lastDot = dot;
			}
		}
	}

	if(choice != myLastFoundInteractable)
	{
		if(myLastFoundInteractable)
		{
			myLastFoundInteractable->HoverLeave();	
		}

		if(choice)
		{
			choice->Hover();
		}

		myLastFoundInteractable = choice;
	}

	return choice;
}

InteractableManager& InteractableManager::Instance()
{
	static InteractableManager instance;
	return instance;
}
