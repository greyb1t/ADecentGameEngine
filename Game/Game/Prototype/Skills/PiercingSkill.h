#pragma once
#include "Skill.h"

class PiercingSkill :
    public Skill
{
public:
    PiercingSkill() = default;
    PiercingSkill(Player*);

	void Activate() override;

	void Update() override;
	void Interrupt() override;
	void Shoot();
private:
	void Charge();
	void ShootProjectile();
private:
	bool myIsActive = false;
	float myCharge = 0;
};
