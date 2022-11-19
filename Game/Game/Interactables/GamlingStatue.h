#pragma once
#include "Engine/GameObject/Components/Component.h"
#include "Game/Components/Interactable.h"

namespace Engine
{
	class AudioComponent;
}

class ItemChoice;

class GamlingStatue final :
	public Interactable
{
public:
	COMPONENT(GamlingStatue, "Gambling Shrine Component");

	void Awake() override;
	void Start() override;

	void Reflect(Engine::Reflector& aReflector) override;

	void Execute(Engine::eEngineOrder aOrder) override;

	void SetIsSandy(bool aIsSandy);

private:
	void UpdatePrice();

	void Gamble();

	void OnHoverEnter();
	void OnHoverExit();

	void SpawnItem();
	void StartWinVFX();
	void StartFailVFX();

	float myItemDropChance = 5.0f;
	float myTryChanceDecrease = 0.9f;

	int myPrice = 40;
	float myTryPirceIncrease = 1.5f;

	float myRareChance = 3.5f;
	float myLegendaryChance = 0.5f;

	float myCantInteractTime = 0.2f;
	float myCantInteractTimer = 0.0f;

	GameObjectPrefabRef myItemChoicePrefab;

	VFXRef myWinVFXRef;
	Engine::VFXComponent* myWinVFX;	
	
	VFXRef myFailVFXRef;
	Engine::VFXComponent* myFailVFX;

	VFXRef myIdleVFXRef;
	Engine::VFXComponent* myIdleVFX;

	Engine::TextComponent* myPriceTag = nullptr;
	float myPriceDistance = 500.0f;
	float myFadeTime = 0.1f;
	float myFadeTimer = 0.0f;
	Vec3f myStartScale;

	bool myIsSandy = false;

	Engine::AudioComponent* myAudio = nullptr;
};
