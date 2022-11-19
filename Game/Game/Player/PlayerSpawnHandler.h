#pragma once
#include "Engine/GameObject/Components/Component.h"
class Player;
class CameraController;
class PlayerSpawnHandler : public Component
{
	COMPONENT(PlayerSpawnHandler, "Player Spawn Handler");
public:
	PlayerSpawnHandler() = default;
	PlayerSpawnHandler(const PlayerSpawnHandler& anOther) = delete;
	PlayerSpawnHandler(GameObject* aGameObject);
	void Init(Player* aPlayer, CameraController* aPlayerCamera);

	void Execute(Engine::eEngineOrder aOrder) override;

private:
	void ReturnControl();
private:
	CU::Vector3f myTargetCameraPosition;
	CU::Vector3f myStartCameraPosition;
	Player* myPlayer = nullptr;
	GameObject* myCamera = nullptr;
	CameraController* myPlayerCamera = nullptr;
	GameObject* myCometObject = nullptr;
	float myZoomCounter = 0.f;
	bool myFirstFrame = true;
	bool myHasArrived = false;

};

