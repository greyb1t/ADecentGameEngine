#include "pch.h"
#include "VFXChild.h"

VFX::VFXChild::VFXChild(const VFXChild&)
{

}

VFX::VFXChild::~VFXChild()
{
	DetachVFX();
}

void VFX::VFXChild::AttachVFX(VFXSystem* aSystem)
{
	mySystem = aSystem;
	mySystem->Attach(this);
}

void VFX::VFXChild::DetachVFX()
{	
	if (!mySystem)
		return;
	mySystem->Detach(this);

	mySystem = nullptr;
}

bool VFX::VFXChild::AutoDestroyCheck()
{
	return true;
}

void VFX::VFXChild::SubscribeStop(const std::function<void()> aFunctíon)
{
	myOnStop = aFunctíon;
}

void VFX::VFXChild::SubscribeForceStop(const std::function<void()> aFunction)
{
	myOnForceStop = aFunction;
}

void VFX::VFXChild::Stop()
{
	if (myOnStop)
		myOnStop();
}

void VFX::VFXChild::ForceStop()
{
	if (myOnForceStop)
		myOnForceStop();
}
