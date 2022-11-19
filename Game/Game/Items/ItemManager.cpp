#include "pch.h"
#include "ItemManager.h"
#include "Item.h"
#include "Engine/GameObject/GameObject.h"

#include "Common/Random.h"

ItemManager::ItemManager()
{
	myItems.reserve(100);
}

ItemManager::~ItemManager()
{
	for (auto* item : myItems)
	{
		delete item;
		item = nullptr;
	}
}

void ItemManager::PostEvent(const eItemEvent& anEvent, const ItemEventData* someData)
{
	if (mySubscribers.find(anEvent) != mySubscribers.end())
	{
		for (auto* item : mySubscribers[anEvent])
		{
			item->RecieveEvent(anEvent, someData);
		}
	}
}

void ItemManager::Subscribe(const eItemEvent& anEvent, Item* aSubscriber)
{
	if (mySubscribers.find(anEvent) == mySubscribers.end())
	{
		mySubscribers.emplace(anEvent, std::vector<Item*>());
	}
	mySubscribers[anEvent].push_back(aSubscriber);
}

void ItemManager::Unsubscribe(const eItemEvent& anEvent, Item* aSubscriber)
{
	if (mySubscribers.find(anEvent) != mySubscribers.end())
	{
		mySubscribers[anEvent].erase(std::remove(mySubscribers[anEvent].begin(), mySubscribers[anEvent].end(), aSubscriber));
	}
	else
	{
		LOG_WARNING(LogType::Items) << "Item " << aSubscriber->GetName() << " attempted to unsubscribe, but it wasn't subscribed :(";
	}
}

Item* ItemManager::GetRandomItem(eRarity aRarity, std::vector<Item*> someForbiddenItems)
{
	if(myItems.empty())
	{
		assert(!"myItems is empty. This should never be the case.");
		return nullptr;
	}

	std::vector<Item*> itemsOfRarity;
	itemsOfRarity.reserve(myItems.size());

	for (auto* item : myItems)
	{
		if (item->GetRarity() == aRarity && std::find(someForbiddenItems.begin(), someForbiddenItems.end(), item) == someForbiddenItems.end())
		{
			itemsOfRarity.push_back(item);
		}
	}
	if (itemsOfRarity.size())
	{
		return itemsOfRarity[Random::RandomUInt(0, itemsOfRarity.size()-1U)];
	}

	LOG_WARNING(LogType::Items) << "Attempted to get an item of " << aRarity._to_string() << " rarity, but no such item could be found :(.";
	std::vector<Item*> items = myItems;

	for (auto* item : someForbiddenItems)
	{
		items.erase(std::remove(items.begin(), items.end(), item));
	}
	if (items.size())
	{
		return items[Random::RandomUInt(0, items.size() - 1U)];
	}
	else
	{
		LOG_ERROR(LogType::Items) << "Could not find a valid item. Returned nullptr.";
		return nullptr;
	}

}

const std::vector<Item*>& ItemManager::GetAllItems() const
{
	return myItems;
}
