#pragma once
class Player;

enum class eCrosshairState
{
	SHOOT,
	RUN,
	SECONDARY_CHARGE,
	SECONDARY_RETURN,
};

class CrossHairHandler
{
public:
	CrossHairHandler(Player* aPlayer = nullptr);

	void Update();

	void HitMark();
	void EnterCharge();
	void ExitCharge();

	float& RotationRef();
	float& InnerRef();
	float& SteadinessOffsetRef();
	float& ChargeRotationSpeedRef();
private:
	void UpdateCharge();

	void UpdateHitMark();
	void EndHitMark();

	void UpdateHitMarkSprite(float aLifeTimePercentage);

private:
	Player* myPlayer = nullptr;
	eCrosshairState myState = eCrosshairState::SHOOT;

	float myRotation = 0;
	float myInner = 7;
	float mySteadinessOffset = 10;
	struct {
		float timer = 0;
		float returnRotation = 0;
	} mySecondary;
	float myChargeSpeed = 360;

	struct {
		bool active = false;
		float timer = 0;
	} myHitmark;
};

