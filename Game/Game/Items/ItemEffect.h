#pragma once
#include "ItemEffectBase.h"
#include "Buff.h"

class HealthComponent;
class Item; 
class ItemEffect : public ItemEffectBase
{
public:
	ItemEffect();
	~ItemEffect(); 

	const eItemEventType GetType() override;

protected:
	void Save(nlohmann::json& aJson) override;
	void Load(nlohmann::json& aJson) override;
	void Expose(int anIndex) override;
private:
	friend class InventoryComponent;
	friend class ItemEditor;

	void ImGuiContextualEffects(int anIndex);

	bool Stacking();

	void ApplyEffect(GameObject& anObject, const ItemEventData* someData) override;
	void ApplyBuff(HealthComponent* aHealthComp);
	std::unique_ptr<Buff> CreateEndBuff(GameObject& anObject);
	std::function<void(GameObject*, float)> GetEndEffect();

	//Specific effects
	void SpawnChainLightning(GameObject& anObject);
	void ApplyDoT(HealthComponent& aHealthComp);

private:
	eItemEffect myEffectType;

	int myTimer = -1;
	float myDuration = -1.f; //Negative duration -> permanent or instant effect
	float myStrength = 10.f;

	float myCustomFloat1 = 1.f;
	float myCustomFloat2 = 1.f;
	float myCustomFloat3 = 1.f;
	float myCustomFloat4 = 1.f;
	float myCustomFloat5 = 1.f;
	int myCustomInt1 = 0;

	int myMaxStacks = 5;
	bool myStackStrength = false;
	bool myStackAppliedEffect = false;
};

