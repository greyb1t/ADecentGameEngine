#pragma once
#include "ItemEffect.h"
#include "Engine/ResourceManagement/ResourceRef.h"
#include "ItemTypes.h"

struct ItemEventData; 


class Item
{
public:
	Item() = default;
	Item(const Item& anOther);
	virtual ~Item();
	virtual void RecieveEvent(const eItemEvent& anEvent, const ItemEventData* someData);
	const std::string& GetName() const;
	const std::string& GetDesc() const;
	const std::string& GetShortDesc() const;
	const std::string GetSpritePath() const;
	const int GetStacks() const;
	const eRarity& GetRarity() const { return myRarity; }
	const uint32_t GetID() { return myID; }

	inline bool operator==(const Item& anOther) const { return myID == anOther.myID; }

private:
	void SetSprite(const std::string& aPath);
	void Start(); //Called from the InventoryComponent. Don't call when creating the item.
	void StopCooldown();

	friend class InventoryComponent;
	friend class ItemEditor;
	TextureRef mySprite;
	std::string myName = "[new item]";
	std::string myDescription = "An Item Description \nSmarrigt";
	std::string myShortDescription = "Short Item Desc";
	eRarity myRarity = eRarity::Common; 
	std::vector<ItemEffectBase*> myEffects;

	float myCooldown = 0.f;
	float myCooldownMultiplier = 1.f;
	float myProcChance = 1.f;
	int myTimerIndex = -1;
	int myStack = 1;
	uint32_t myID = 0;

	bool myStackProcChance = false;
	bool myIsOnCooldown = false;
	bool myIsActive = false;
};

