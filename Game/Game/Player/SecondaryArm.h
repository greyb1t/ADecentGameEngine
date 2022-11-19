#pragma once

class Player;

namespace Engine
{
	class ModelComponent;
	class VFXComponent;
}

class SecondaryArm
{
public:
	void Init(Player* aPlayer);
	void Update();

	void ActivateSecondary();
	void DisableArm();

	GameObject* GetForearm() const;
private:
	void UpdatePosition();
	void UpdateCharge();
private:
	Player* myPlayer = nullptr;
	GameObject* myForearm = nullptr;
	Engine::VFXComponent* mySecondaryChargeVFX = nullptr;

	int myArmBoneIndex = -1;
	int myHandBoneIndex = -1;

	bool myIsActive = false;
};

