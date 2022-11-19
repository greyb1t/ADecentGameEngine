#pragma once
#include "Skill.h"
class PistolSkill :
    public Skill
{
public:
	PistolSkill() = default;
	PistolSkill(Player*);

	void Activate() override;
	void Shoot();
private:
};

