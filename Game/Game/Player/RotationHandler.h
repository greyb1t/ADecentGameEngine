#pragma once
class Player;
class RotationHandler
{
public:
	void Init(Player* aPlayer);
	void Update();
private:
	CU::Quaternion myRotation;
	Player* myPlayer = nullptr;
	float myCatchUpPerSecond = 9.0f;
	float myCombatCatchUpPerSecond = 9.f;
	float myMaxAngleDif = .1f;
};

