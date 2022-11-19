#include "pch.h"
#include "Enum.h"

int Engine::Enum::GetValue() const
{
	return myValue;
}

void Engine::Enum::SetValue(int aValue)
{
	myValue = aValue;
}
