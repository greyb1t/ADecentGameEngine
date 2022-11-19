#pragma once
#include "Engine\GameObject\Components\Component.h"
#include "Engine\ResourceManagement\ResourceRef.h"

namespace Engine
{
	class VFXComponent;
}

class HealthComponent;

class MoneyDropComponent : public Component
{
	COMPONENT(MoneyDropComponent, "Money Drop Component");
public:
	void Start() override;
	void Reflect(Engine::Reflector& aReflector) override;

	void SetMoneyDropAmmount(int aAmmount);

	void ZeroMoneyDrop();

private:
	void OnDeath();

	int myMoneyDrop = 0;

	HealthComponent* myHealthComponent = nullptr;

	Engine::VFXComponent* myMoneyVFX = nullptr;
	VFXRef myMoneyRef;
};

