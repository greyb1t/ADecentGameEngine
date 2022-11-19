#pragma once
#include "VFXSystem.h"

namespace VFX
{
	class VFXChild
	{
	public:
		VFXChild() = default;
		VFXChild(const VFXChild&);
		virtual ~VFXChild();

		void AttachVFX(VFXSystem* aSystem);
		void DetachVFX();

	protected:
		virtual bool AutoDestroyCheck();

		void SubscribeStop(const std::function<void()> aFunction);
		void SubscribeForceStop(const std::function<void()> aFunction);
	private:
		friend class VFXSystem;
		void Stop();
		void ForceStop();

		VFXSystem* mySystem = nullptr;
		std::function<void()> myOnStop;
		std::function<void()> myOnForceStop;
	};

}