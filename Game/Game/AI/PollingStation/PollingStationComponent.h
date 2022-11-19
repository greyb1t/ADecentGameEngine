#pragma once
#include "Engine/GameObject/Components/Component.h"

class PollingStationComponent : public Component
{
public:
	COMPONENT_SINGLETON(PollingStationComponent, "PollingStation");

	PollingStationComponent() = default;
	PollingStationComponent(GameObject* aGameObject);
	~PollingStationComponent() = default;

	void Start() override;

	void Reflect(Engine::Reflector& aReflector) override;

	void Execute(Engine::eEngineOrder aOrder) override;
	void Render() override;

	void Setplayer(Component* aComponent);
	const Vec3f GetPlayerPos();
	const Vec3f GetDelayedPlayerPos(int aDelayValue);
	GameObject* GetPlayer();
	Component* GetPlayerComponent();

	const std::vector<GameObject*>& GetEnemies();

	void DebugDraw();

	void EnemyCounterIncrease();
	void EnemyCounterDecrease();
	void UpdateMusicEnemyCount();
	int GetNumberOfEnemiesInLevel();
	int GetMAXNumberOfEnemiesInLevel();

	// LIGHT POOL
	void CreatePointLightPool();
	// returns nullptr if pool is empty
	GameObject* GetPointlightFromPool();
	void ReturnPointLightToPool(GameObject* aPointLightGameObject);

private:
	void IncrementDelayedPositions();

	Component* myPlayerComponent = nullptr;
	std::vector<Vec3f> myDelayedPlayerPositions;
	int myNumberOfEnemies = 0;
	int myMaxNumberOfEnemies = 50;

	// LIGHT POOL
	constexpr static inline int ourMaxPointLightsInPool = 40;
	C::VectorOnStack<GameObject*, ourMaxPointLightsInPool> myPointLightPool;
	std::queue<GameObject*> myPoolQueue;
};
