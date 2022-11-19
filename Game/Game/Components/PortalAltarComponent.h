#pragma once
#include <Game/Components/Interactable.h>
#include "Engine\GameObject\Components\Component.h"
#include "Engine/GeneralEventManager/GeneralEventListener.h"
#include "Engine/GeneralEventManager/GeneralEventManager.h"

class PortalAltarComponent : public Interactable
{
public:
	COMPONENT(PortalAltarComponent, "Portal Altar Component");

	~PortalAltarComponent();

	void Start() override;

	void Reflect(Engine::Reflector& aReflector) override;

	void SetEvent(GeneralEvent aEvent);
	GeneralEvent GetEvent() { return myEvent; }

	void SpawnPortal();

	void Execute(Engine::eEngineOrder aOrder) override;

protected:
	
private:
	GeneralEvent myEvent;
	float myTimer = 0;
	float myPortalSpawnTime = 1.0f;
	int myGoldAmount = 0;
	bool myTimerStarted = false;
};