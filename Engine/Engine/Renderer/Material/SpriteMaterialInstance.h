#pragma once

#include "MaterialInstance.h"

namespace Engine
{
	class SpriteMaterialInstance : public MaterialInstance
	{
	public:

	protected:
		void ReflectMaterial(Reflector& aReflector);
	};
}