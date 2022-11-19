#include "pch.h"
#include "BodyIdle.h"
#include "AI/FinalBoss/Constants.h"
#include "Engine/GameObject/Components/AnimatorComponent.h"

FB::BodyIdle::BodyIdle(FinalBossBody& aBody)
	: myBody(aBody)
{
}

void FB::BodyIdle::Update()
{
}

bool FB::BodyIdle::IsFinished() const
{
	return false;
}

void FB::BodyIdle::OnEnter()
{
}

void FB::BodyIdle::OnExit()
{
}

void FB::BodyIdle::Reflect(Engine::Reflector& aReflector)
{
}

bool FB::BodyIdle::IsAllowedToInterrupt() const
{
	return true;
}
