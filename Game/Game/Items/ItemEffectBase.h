#pragma once
#include "ItemTypes.h"

BETTER_ENUM(eItemEventType, unsigned short,
	Base,
	VFX
)

class Item;
class ItemEffectBase
{
public:
	ItemEffectBase();
	virtual ~ItemEffectBase() = default;

	bool ResolveEffect(const ItemEventData* someData, bool aHasProcced);
	const eItemEvent& GetEvent() const;

	virtual const eItemEventType GetType() = 0;

	virtual void Save(nlohmann::json& aJson);
	virtual void Load(nlohmann::json& aJson);
	virtual void Expose(int anIndex);
	void SetItem(Item* anItem);
	void SetIndex(uint32_t anIndex);

protected:
	friend class InventoryComponent;

	virtual void ApplyEffect(GameObject& anObject, const ItemEventData* someData) = 0;

	Item* myItem = nullptr;
	uint32_t myIndex = 0U;
private:
	eItemEvent myTriggeringEvent;
	eItemTarget myTarget;

	float myAOE = 100.f;


	bool myShouldProc = false;
	bool myTriggersCooldown = true;

};