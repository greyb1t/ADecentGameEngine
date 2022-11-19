#pragma once
#include "Engine/GameObject/Components/Component.h"

class ChestSpawnerComponent : public Component
{
	COMPONENT(ChestSpawnerComponent, "Chest Spawner");
public:
	void Start();

	void Execute(Engine::eEngineOrder anOrder) override;
	void Reflect(Engine::Reflector& aReflector) override;

private:
	const CU::Quaternion CreateRotationFromUp(const CU::Vector3f& anUpDirection) const;

private:
	int myNumberOfChests = 10;
	float myChestMinDistance = 2000.f;	//Centimeters
	float myRareChestPercent = 0.1;
	GameObjectPrefabRef myChestPrefab;
	GameObjectPrefabRef myChestRarePrefab;
	GameObjectPrefabRef myShrinePrefab;
	bool myHasStarted = false;

	bool myHasSandyShrine = false;
};

