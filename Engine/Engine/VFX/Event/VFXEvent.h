#pragma once
#include "pch.h"

class VFXSystem;
class Transform;

enum class eVFXEvent
{
	EMITTER,
};

class VFXEvent
{
public:
	VFXEvent();
	virtual void Activate() = 0;

	void SetVFX(VFXSystem* aVFX);
protected:
	VFXSystem* myVFX = nullptr;
	eVFXEvent myType = eVFXEvent::EMITTER;
};