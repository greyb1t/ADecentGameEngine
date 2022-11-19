#include "pch.h"
#include "Emitter.h"
#include "../Description/EmitterDescription.h"

VFX::Emitter::Emitter(const EmitterDescription& data)
{
	Init(data);
}

void VFX::Emitter::Init(const EmitterDescription& description)
{
	myDescription = description;
	myDescription.timeline.Reset();
	myDescription.timeline.SetTotalTime(myDescription.duration);
	myTimer = 0;
}

void VFX::Emitter::Tick()
{
	if (myState != eVFXState::PLAY)
		return;

	if (const unsigned emits = myDescription.timeline.Tick(Time::DeltaTime / myDescription.duration))
	{
		for (int i = 0; i < emits; ++i)
		{
			Emit();
		}
	}


	myTimer += Time::DeltaTime;
	if (myTimer > myDescription.duration)
	{
		Stop();
	}
}

void VFX::Emitter::Stop()
{
	myState = eVFXState::STOP;
}

void VFX::Emitter::SetTimeline(Timeline& aTimeline)
{
	myDescription.timeline = aTimeline;
}

void VFX::Emitter::SetDuration(float aDuration)
{
	myDescription.duration = aDuration;
}
