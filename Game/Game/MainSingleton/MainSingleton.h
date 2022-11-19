#pragma once
#include <memory>
#include "Items/Item.h"

class TimerManager;
class ItemManager;
class GameObject;
class Game;
class ItemEditor;
class StatsDisplay;
struct RorPlayerStats;

namespace NavMesh
{
	class NavMesh;
}

class PlayerCharacter;

enum class eSceneID
{
	Level1,
	Level2,
	Level3,
	Level4,
	Level5,
	Level6,
	Level7,
	Level8,

	COUNT
};

class MainSingleton
{
public:
	static bool Start();
	static void Destroy();
	static void Update();
	static bool Exists();
	~MainSingleton(); 
	MainSingleton(MainSingleton const&) = delete;
	void operator=(MainSingleton const&) = delete;

	static TimerManager& GetTimerManager();
	static ItemManager& GetItemManager();

	static void SaveProgress();
	static void ClearProgress();
	static void SavePlayerItems();
	static const std::vector<std::pair<Item, int>>& GetPlayerItems();

	static void SavePlayerMaxHP();
	static void LoadPlayerHP();

	static StatsDisplay& GetStatsDisplay();

	static GameObject* GetPlayer();

	//Returns true if the player should clear its items
	static bool SetPlayer(GameObject* anObject); 

	static eSceneID GetSceneID();
	static void CueResetPlayer();

	static void AddPlayerShootObserver(int aID, const std::function<void()>& aCallback);
	static void RemovePlayerShootObserver(int aID);
	static void PlayerShootEvent();

	static bool GetCanOpenChestShield();
	static void SetCanOpenChestShield(bool aCanOpen);

private:
	MainSingleton() = default;
	bool InternalStart();

	static void	ClearPlayerItems();


	TimerManager* myTimerManager = nullptr;
	ItemManager* myItemManager = nullptr;
	Owned<ItemEditor> myItemEditor;
	Owned<StatsDisplay> myStatsDisplay;
	std::vector<std::pair<Item, int>> myPlayerItems;

	GameObject* myPlayer = nullptr;
	eSceneID mySceneID = eSceneID::COUNT;

	float myPlayerBaseMaxHP = -1.f;
	float myPlayerMaxHPMod = -1.f;

	bool myShouldResetPlayer = false;

	std::unordered_map<int, std::function<void()>> myPlayerShootObservers;

	bool myCanOpenChestShields = true;
};

using Main = MainSingleton;

