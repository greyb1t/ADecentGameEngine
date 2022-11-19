#pragma once

#include "MaterialInstance.h"

namespace Engine
{
	class ParticleMaterialInstance : public MaterialInstance
	{
	public:

	protected:
		void ReflectMaterial(Reflector& aReflector);
	};
}