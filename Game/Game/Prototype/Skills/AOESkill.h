#pragma once
#include "Skill.h"

class AOESkill :
    public Skill
{
public:
	AOESkill();
	AOESkill(Player*);

	void Activate() override;

	void Shoot();

	inline void SetDamage(float aDamage) { myDamage = aDamage; }
	inline void SetBounces(int someBounces) { myBounces = someBounces; }

private:
	float myDamage = 20;
	int myBounces = 6;
	float myBounceRange = 800;
	float myProjectileSpeed = 1000;
	float myProjectileRange = 10000;
};

