#pragma once
#include "EmitterGeometry.h"
#include "Engine/GameObject/Transform.h"
#include "Engine/VFX/VFXEnums.h"
#include "Engine/VFX/Description/EmitterDescription.h"
#include "Engine/VFX/Time/Timeline.h"

namespace VFX
{
	struct EmitterDescription;

	class Emitter
{
public:
	Emitter() = default;
	Emitter(const EmitterDescription& data);
	virtual ~Emitter() = default;
	void Init(const EmitterDescription& description);

	virtual void Tick();

	virtual void Emit() = 0;

	void Stop();

	void SetTimeline(Timeline& aTimeline);
	void SetDuration(float aDuration);

	bool IsFinished() const { return myState == eVFXState::STOP; }
	
	Timeline& GetTimeline() { return myDescription.timeline; }
	EmitterDescription& GetDescription() { return myDescription; }
protected:
	EmitterDescription myDescription;

	float myTimer = 0.f;
	eVFXState myState = eVFXState::PLAY;
};
}