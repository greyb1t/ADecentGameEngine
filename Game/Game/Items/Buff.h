#pragma once
#include "ItemTypes.h"

struct BuffID
{
	BuffID(uint32_t anItemID, uint32_t anEffectIndex) : 
		itemID(anItemID), effectIndex(anEffectIndex)
	{ }
	inline bool operator==(const BuffID& anOther) const { return itemID == anOther.itemID && effectIndex == anOther.effectIndex; }
	inline bool operator<(const BuffID& anOther) const { return itemID < anOther.itemID && effectIndex < anOther.effectIndex; }

	uint32_t itemID = 0;
	uint32_t effectIndex = 0;
};

namespace std
{
	template <>
	struct hash<BuffID>
	{
		std::size_t operator()(const BuffID& anID) const
		{
			size_t hashRes = static_cast<size_t>(anID.itemID);
			hashRes = hashRes << 32;
			hashRes += static_cast<size_t>(anID.effectIndex);
			return hashRes;
		}
	};
}

class Buff
{
public:
	Buff(const std::function<void(GameObject*, float)>& anEndFunction, GameObject& aGameObject, BuffID aBuffID, float aDuration);
	Buff(const std::function<void(GameObject*, float)>& aTickFunction, GameObject& aGameObject, BuffID aBuffID, float aDuration, float aTickTime);
	~Buff() = default;

	bool Update(float aDeltaTime);
	bool IsValid() const { return myFunction != nullptr; };
	void AddStack();
	void Refresh(float aDuration);
	int GetStacks() { return myStacks; }
	inline const BuffID GetID() { return myBuffID; }
	float GetDuration() { return myRemainingTime; }
private:
	void Callback();

	GameObject* myGameObject = nullptr;
	std::function<void(GameObject*, float)> myFunction;
	BuffID myBuffID;
	float myRemainingTime = 0.f;
	float myTickTime = 1.f;
	float myTickTimer = 1.f;
	int myStacks = 1;
	bool myCallsOnTicks = false;
};

