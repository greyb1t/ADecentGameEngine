#pragma once
#include <Game/Components/Interactable.h>
#include "Engine\GameObject\Components\Component.h"
#include "Engine/GeneralEventManager/GeneralEventListener.h"
#include "Engine/GeneralEventManager/GeneralEventManager.h"
#include <Engine/Scene/SceneManager.h>
#include "../Game/Game/Scenes/GameScene.h"

class PortalComponent : public Interactable
{
public:
	COMPONENT(PortalComponent, "Portal Component");
	~PortalComponent();

	void Start() override;

	void Reflect(Engine::Reflector& aReflector) override;

	void SetEvent(GeneralEvent aEvent);
	GeneralEvent GetEvent() { return myEvent; }

	void SpawnPortal();

	void StartLoadLevel();

	void Execute(Engine::eEngineOrder aOrder) override;

protected:
	
private:
	GeneralEvent myEvent;
	std::string myNextLevelSceneName = "";
	std::string myNextLevelFolderName = "";
	float myTimer = 0;
	float myNextLevelTime = 1.5f;
	int myGoldAmount = 0;
	bool myTimerStarted = false;
	Vec3f myRot;
	Vec3f myRotSpeed;
};