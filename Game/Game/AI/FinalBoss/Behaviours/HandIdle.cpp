#include "pch.h"
#include "HandIdle.h"

FB::HandIdle::HandIdle()
{
}

void FB::HandIdle::Update()
{
}

bool FB::HandIdle::IsFinished() const
{
	return false;
}

void FB::HandIdle::OnEnter()
{
}

void FB::HandIdle::OnExit()
{
}

void FB::HandIdle::Reflect(Engine::Reflector& aReflector)
{
}

bool FB::HandIdle::IsAllowedToInterrupt() const
{
	return true;
}
