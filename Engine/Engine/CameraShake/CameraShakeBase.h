#pragma once
#include "Displacement.h"

namespace Engine
{
	class Camera;
}

namespace Engine
{
	class CameraShakeBase
	{
	public:
		virtual ~CameraShakeBase() = default;

		virtual void Update(const float aDeltaTime) = 0;

		bool IsFinished() const;

		const Displacement& GetDisplacement() const;

	protected:
		bool myIsFinished = false;
		Displacement myDisplacement;
	};
}