#pragma once
class Player;

class Skill
{
public:
	Skill() = default;
	Skill(Player* aPlayer);

	virtual void Update();

	virtual bool CanActivate();
	virtual void Activate();
	virtual void Interrupt();

	void SetCooldown(float aTime);
	float GetCooldown() const;

protected:
	void UpdateCooldown();
	void ResetCooldown();

protected:
	Player* myPlayer = nullptr;

	bool myIsOnCoolDown = false;
	float myCooldown = 1;
	float myTimer = 0;
};

