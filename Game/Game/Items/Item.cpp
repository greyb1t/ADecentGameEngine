#include "pch.h"
#include "Item.h"
#include "Components/HealthComponent.h"
#include "Engine/GameObject/GameObject.h"
#include "MainSingleton/MainSingleton.h"
#include "Engine/Utils/TimerManager.h"
#include "Engine/ResourceManagement/ResourceManager.h"
#include "Engine/ResourceManagement/Resources/TextureResource.h"
#include "ItemManager.h"
#include "ItemVFX.h"
#include <Player/Player.h>

Item::Item(const Item& anOther)
{
	myName = anOther.myName;
	myDescription = anOther.myDescription;
	myShortDescription = anOther.myShortDescription;
	myCooldown = anOther.myCooldown;
	myProcChance = anOther.myProcChance;
	myStackProcChance = anOther.myStackProcChance;
	mySprite = anOther.mySprite;
	myStack = anOther.myStack;
	myID = anOther.myID;
	for (int i = 0; i < anOther.myEffects.size(); ++i)
	{
		switch (anOther.myEffects[i]->GetType())
		{
		case eItemEventType::Base:
			myEffects.push_back(DBG_NEW ItemEffect(*static_cast<ItemEffect*>(anOther.myEffects[i])));
			break;
		case eItemEventType::VFX:
			myEffects.push_back(DBG_NEW ItemVFX(*static_cast<ItemVFX*>(anOther.myEffects[i])));
			break;
		}
	}

	myIsActive = false;
	myIsOnCooldown = false;
	myTimerIndex = -1;
}

Item::~Item()
{
	if (myIsActive && Main::Exists())
	{
		std::vector<eItemEvent> subscribedEvents;
		for (auto* effect : myEffects)
		{
			const eItemEvent& itemEvent = effect->GetEvent();
			if (std::find(subscribedEvents.begin(), subscribedEvents.end(), itemEvent) == subscribedEvents.end())
			{
				subscribedEvents.push_back(itemEvent);
				Main::GetItemManager().Unsubscribe(itemEvent, this);
			}
		}
		Main::GetTimerManager().StopTimer(myTimerIndex);
	}

	for (auto* effect : myEffects)
	{
		delete effect;
		effect = nullptr;
	}
}

void Item::RecieveEvent(const eItemEvent& anEvent, const ItemEventData* someData)
{
	if (!myIsOnCooldown)
	{
		bool hasProcced = Random::RandomFloat(0.f, 1.f) < myProcChance;
		bool goOnCooldown = false;
		for (auto& effect : myEffects)
		{
			if (effect->GetEvent() == anEvent)
			{
				if (effect->ResolveEffect(someData, hasProcced))
				{
					goOnCooldown = true;
				}
			}
		}
		if (goOnCooldown && (myCooldown * Main::GetPlayer()->GetComponent<Player>()->GetStats().GetCooldownMultiplier()) > 0)
		{
			myIsOnCooldown = true;
			myTimerIndex = Main::GetTimerManager().SetTimer(myCooldown, [this]() {StopCooldown(); });
		}
	}
}

const std::string& Item::GetName() const
{
	return myName;
}

const std::string& Item::GetDesc() const
{
	return myDescription;
}

const std::string& Item::GetShortDesc() const
{
	return myShortDescription;
}

const std::string Item::GetSpritePath() const
{
	if(mySprite)
	{
		return mySprite->GetPath();
	}

	return "";
}

const int Item::GetStacks() const
{
	return myStack;
}

void Item::SetSprite(const std::string& aPath)
{
	TextureRef textureResource =
		GetEngine().GetResourceManager().CreateRef<Engine::TextureResource>(aPath);
	assert(textureResource->GetState() == ResourceState::Loaded);

	mySprite = textureResource;
}

void Item::StopCooldown()
{
	myIsOnCooldown = false;
}

void Item::Start()
{
	std::vector<eItemEvent> subscribedEvents;
	subscribedEvents.push_back(eItemEvent::OnPickup);
	for (auto* effect : myEffects)
	{
		const eItemEvent& itemEvent = effect->GetEvent();
		if (std::find(subscribedEvents.begin(), subscribedEvents.end(), itemEvent) == subscribedEvents.end())
		{
			subscribedEvents.push_back(itemEvent);
			Main::GetItemManager().Subscribe(itemEvent, this);
		}

		if (itemEvent == +eItemEvent::OnPickup)
		{
			effect->ResolveEffect(nullptr, true);
		}
	}
	myIsActive = true;
}

