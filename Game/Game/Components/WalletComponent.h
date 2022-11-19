#pragma once
#include "Engine\GameObject\Components\Component.h"

class HUDHandler;

class WalletComponent : public Component
{
public:
	COMPONENT(WalletComponent, "Wallet Component");
	
	void Start() override;

	void Reflect(Engine::Reflector& aReflector) override;

	bool TryBuy(int aItemPrice);

	void DecreaseGold(int aAmount);
	void IncreaseGold(int aAmount);

protected:
	
private:
	HUDHandler* myHUD = nullptr;
};