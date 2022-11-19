#pragma once
#include "ItemEffectBase.h"
class ItemVFX : public ItemEffectBase
{
public:
	const eItemEventType GetType() override;

	void Save(nlohmann::json& aJson) override;
	void Load(nlohmann::json& aJson) override;
	void Expose(int anIndex) override;

private:
	void ApplyEffect(GameObject& anObject, const ItemEventData* someData) override;
	void PlayVFX(Weak<GameObject> anObject, const CU::Vector3f& aPosition);
	void SpawnPrefab(Weak<GameObject> anObject, const CU::Vector3f& aPosition);

private:
	VFXRef myVFX;
	std::string myVFXPath = "";
	CU::Vector3f myPositionOffset;
	float myDelay = 0.f;
	float myDuration = -1.f;
	bool myShouldFollowTarget = false;
	bool myUsePrefab = false;
};

