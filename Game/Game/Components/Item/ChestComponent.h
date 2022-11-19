#pragma once

#include "Game/Components/Interactable.h"
#include "Game/Items/ItemTypes.h"

#include "Engine/ResourceManagement/ResourceRef.h"
#include "Engine/ResourceManagement/Resources/GameObjectPrefabResource.h"

namespace Engine
{
	class AnimationController;
	class VFXComponent;
	class AudioComponent;
}

class PlayerShield;
class ItemChoice;

class ChestComponent final :
	public Interactable
{
	COMPONENT(ChestComponent, "ChestComponent");

public:
	void Awake() override;
	void Start() override;

	void Reflect(Engine::Reflector& aReflector) override;

	void Execute(Engine::eEngineOrder aOrder) override;

	void OnItemSelected(Item* aItem);

	int GetPrice();

private:
	void UpdatePrice();

	void Open();

	void OnHoverEnter();
	void OnHoverExit();

	void FinishedOpening();
	void StartVFX();
	void RandomizeItems();

	void OpenChestSound(eRarity aRarity);

	eRarity GetRandomRarity();

	GameObjectPrefabRef myItemChoicePrefab;
	Weak<GameObject> myBeamObject;

	GameObjectPrefabRef myShieldPrefab;
	PlayerShield* myShield = nullptr;

	VFXRef myVFXRef;
	Engine::VFXComponent* myVFX;
	Vec3f myVFXRotation;

	Engine::TextComponent* myText = nullptr;

	Engine::TextComponent* myPriceTag = nullptr;
	float myPriceDistance = 500.0f;
	float myFadeTime = 0.1f;
	float myFadeTimer = 0.0f;
	bool myShouldUpdatePrice = true;
	Vec3f myStartScale;

	float myDeclineTime = 0.3f;
	float myDeclineTimer = 0.0f;

	ItemChoice* myFirstObject = nullptr;
	ItemChoice* mySecondObject = nullptr;
	ItemChoice* myThirdObject = nullptr;
	
	eRarity myRarity = eRarity::Common;

	Engine::AnimationController* myAnimation = nullptr;

	std::array<float, eRarity::_size()> myRandomValues{1.f, 1.f, 1.f};

	int myPrice = 50;
	bool myIsOpened = false;
	bool myHasStartedOpenening = false;

	bool myIsRandomDrop = true;
	float myRarePriceMultiplier = 1.0f;

	Engine::AudioComponent* myAudio = nullptr;
};