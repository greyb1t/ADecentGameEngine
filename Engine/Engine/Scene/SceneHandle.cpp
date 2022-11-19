#include "pch.h"
#include "SceneHandle.h"

Engine::SceneHandle::SceneHandle(const uint32_t aValue) : myValue(aValue)
{
}

bool Engine::SceneHandle::IsValid() const
{
	return myValue != static_cast<uint32_t>(-1);
}
