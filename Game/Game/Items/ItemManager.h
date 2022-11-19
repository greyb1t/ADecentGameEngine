#pragma once

#include <map>

#include "ItemTypes.h"

class GameObject;
class Item;

class ItemManager
{
public:
	ItemManager();
	~ItemManager();

	void PostEvent(const eItemEvent& anEvent, const ItemEventData* someData);
	void Subscribe(const eItemEvent& anEvent, Item* aSubscriber);
	void Unsubscribe(const eItemEvent& anEvent, Item* aSubscriber);

	Item* GetRandomItem(eRarity aRarity, std::vector<Item*> someForbiddenItems);
	const std::vector<Item*>& GetAllItems() const;

private:
	friend class ItemEditor;

	ItemManager(const ItemManager& anOther) = delete;
	std::map<eItemEvent::_enumerated, std::vector<Item*>> mySubscribers;

	std::vector<Item*> myItems;
};

