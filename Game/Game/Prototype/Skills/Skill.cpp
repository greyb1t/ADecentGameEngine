#include "pch.h"
#include "Skill.h"

Skill::Skill(Player* aPlayer)
	: myPlayer(aPlayer)
{
}

void Skill::Update()
{
	UpdateCooldown();
}

void Skill::UpdateCooldown()
{
	if (myIsOnCoolDown)
	{
		myTimer -= Time::DeltaTime;
		if (myTimer <= 0)
		{
			myTimer = 0;
			myIsOnCoolDown = false;
		}
	}
}

void Skill::ResetCooldown()
{
	myTimer = myCooldown;
	myIsOnCoolDown = true;
}

bool Skill::CanActivate()
{
	return !myIsOnCoolDown;
}

void Skill::Activate()
{
	ResetCooldown();
}

void Skill::Interrupt()
{
}

void Skill::SetCooldown(float aTime)
{
	myCooldown = aTime;
}

float Skill::GetCooldown() const
{
	return myCooldown;
}
