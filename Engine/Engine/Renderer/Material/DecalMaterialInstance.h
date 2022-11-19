#pragma once

#include "MaterialInstance.h"

namespace Engine
{
	class DecalMaterialInstance : public MaterialInstance
	{
	public:

	protected:
		void ReflectMaterial(Reflector& aReflector);
	};
}