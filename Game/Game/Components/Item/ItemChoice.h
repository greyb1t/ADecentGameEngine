#pragma once

#include "Game/Components/Interactable.h"
#include "Engine/GameObject/Components/TextComponent.h"

class ChestComponent;
class Item;

class HUDItemDescription;

class ItemChoice final :
	public Interactable
{
	COMPONENT(ItemChoice, "ItemChoice");
	
public:
	void Awake() override;
	void Start() override;

	void Execute(Engine::eEngineOrder aOrder) override;

	void Open(const uint32_t aIndex, ChestComponent* aChest, const Vec3f& aOriginalPosition, const Vec3f& aTargetPosition);
	void Close();

	void HoverEnter();
	void HoverExit();
	
	void SetTargetPosition(const Vec3f aPosition);
	void SetItem(Item* anItem);

	void DestroyOnSelect();

	void SetLerpSpeed(float aLerpSpeed/* = 5.0f*/);

	void SetPontusBoolToTrue();

private:
	void Select();

	uint32_t myIndex = 0;
	ChestComponent* myChest = nullptr;
	Vec3f myOriginalPosition{};
	Vec3f myTargetPosition{};
	float myLerpAmount = 0.f;
	float myHoverLerpAmount = 0.f;
	float myHoverLerpDirection = -1.f;
	Item* myItem = nullptr;
	bool myWasSelected = false;
	bool myIsReturning = false;

	bool myPontusBool = false;

	float myLerpSpeed = 5.0f;

	float myScale = 0.25f;

	HUDItemDescription* myHUDItemDescription = nullptr;

	bool myDestroyOnSelect = false;
};