#include "pch.h"
#include "MainSingleton.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/Navmesh/NavMeshThreaded.h"
#include "Engine/Utils/TimerManager.h"
#include "Items/ItemManager.h"
#include "Items/ItemEditor.h"
#include <stack>
#include "Engine/Editor/EditorWindows/Inspector/InspectorWindow.h"
#include "Engine/ResourceManagement/ResourceManager.h"
#include "Engine/Shortcuts.h"
#include "imgui.h"
#include "Engine/Editor/Editor.h"
#include "Game/Player/Player.h"
#include "UI/PauseMenu/StatsDisplay.h"
#include "Engine/UIEventManager/UIEventManager.h"
#include "Components/InventoryComponent.h"
#include "Components/HealthComponent.h"

static MainSingleton* myInstance;

bool MainSingleton::Start()
{
	if (!myInstance)
	{
		myInstance = new MainSingleton();
		return myInstance->InternalStart();
	}

	return false;
}

void MainSingleton::Destroy()
{
	delete myInstance;
	myInstance = nullptr;
}

void MainSingleton::Update()
{
	myInstance->myTimerManager->Update();
	using UI = UIEventManager;
	UI::Update();
}

bool MainSingleton::Exists()
{
	return myInstance; 
}

MainSingleton::~MainSingleton()
{
	myPlayerItems.clear();

	delete myItemManager;
	myItemManager = nullptr;

	delete myTimerManager;
	myTimerManager = nullptr;
}

TimerManager& MainSingleton::GetTimerManager()
{
	return *myInstance->myTimerManager; 
}

ItemManager& MainSingleton::GetItemManager()
{
	return *myInstance->myItemManager;
}

void MainSingleton::SaveProgress()
{
	SavePlayerMaxHP();
	SavePlayerItems(); 
}

void MainSingleton::ClearProgress()
{
	ClearPlayerItems(); 
	myInstance->myPlayerBaseMaxHP = -1.f;
	myInstance->myPlayerMaxHPMod = -1.f;
}

void MainSingleton::SavePlayerItems()
{
	if (myInstance)
	{
		const std::vector<Item*>& items = Main::GetPlayer()->GetComponent<InventoryComponent>()->GetItems();
		myInstance->myPlayerItems.clear();
		myInstance->myPlayerItems.reserve(items.size());
		for (auto* item : items)
		{
			myInstance->myPlayerItems.push_back(std::make_pair(*item, item->GetStacks()));
		}
		LOG_INFO(LogType::Bjorn) << std::to_string(items.size()) << " items saved from player.";

	}
	else
	{
		LOG_ERROR(LogType::Game) << "COULD NOT SAVE ITEMS";
	}
}

const std::vector<std::pair<Item, int>>& MainSingleton::GetPlayerItems()
{
	return myInstance->myPlayerItems;
}

void MainSingleton::ClearPlayerItems()
{
	myInstance->myPlayerItems.clear();
}

void MainSingleton::SavePlayerMaxHP()
{
	if (Main::GetPlayer())
	{
		HealthComponent* playerHealth = Main::GetPlayer()->GetComponent<HealthComponent>();
		playerHealth->ClearBuffs();
		myInstance->myPlayerBaseMaxHP = playerHealth->GetMaxHealthBase();
		myInstance->myPlayerMaxHPMod = playerHealth->GetMaxHealthMod();
	}
}

void MainSingleton::LoadPlayerHP()
{
	if (myInstance->myPlayerBaseMaxHP > 0.f)
	{
		HealthComponent* playerHealth = Main::GetPlayer()->GetComponent<HealthComponent>();
		playerHealth->SetMaxHealth(myInstance->myPlayerBaseMaxHP);
		playerHealth->SetMaxHealthModifier(myInstance->myPlayerMaxHPMod);
	}

}

StatsDisplay& MainSingleton::GetStatsDisplay()
{
	return *myInstance->myStatsDisplay; 
}

GameObject* MainSingleton::GetPlayer()
{
	if (!myInstance->myPlayer)
	{
		LOG_WARNING(LogType::Game) << "Attempted to fetch player, but it was nullptr";

	}
	return myInstance->myPlayer;
}

bool MainSingleton::SetPlayer(GameObject* anObject)
{
	myInstance->myPlayer = anObject;

	if (myInstance->myPlayer && myInstance->myShouldResetPlayer)
	{
		myInstance->myShouldResetPlayer = false;
		return true;
	}
	return false;
}

eSceneID MainSingleton::GetSceneID()
{
	return myInstance->mySceneID; 
}

void MainSingleton::CueResetPlayer()
{
	myInstance->myShouldResetPlayer = true;
}

void MainSingleton::AddPlayerShootObserver(int aID, const std::function<void()>& aCallback)
{
	myInstance->myPlayerShootObservers[aID] = aCallback;
}

void MainSingleton::RemovePlayerShootObserver(int aID)
{
	auto& observers = myInstance->myPlayerShootObservers;

	if (observers.find(aID) != observers.end())
	{
		observers[aID] = nullptr;
		observers.erase(aID);
	}
}

void MainSingleton::PlayerShootEvent()
{
	for (auto observer : myInstance->myPlayerShootObservers)
	{
		if (observer.second)
		{
			observer.second();
		}
	}
}

bool MainSingleton::GetCanOpenChestShield()
{
	return myInstance->myCanOpenChestShields;
}

void MainSingleton::SetCanOpenChestShield(bool aCanOpen)
{
	myInstance->myCanOpenChestShields = aCanOpen;
}

bool MainSingleton::InternalStart()
{
	myTimerManager = new TimerManager();
	myItemManager = new ItemManager(); 
	myItemEditor = MakeOwned<ItemEditor>(); 
	myStatsDisplay = MakeOwned<StatsDisplay>();
	if (GetEngine().IsEditorEnabled())
	{
		GetEngine().GetEditor().AddExternalTool(std::function<void(void)>(std::bind(&ItemEditor::Update, myItemEditor.get())));
		GetEngine().GetEditor().AddExternalTool(std::function<void(void)>(std::bind(&StatsDisplay::Update, myStatsDisplay.get())));
	}

	return true;
}

