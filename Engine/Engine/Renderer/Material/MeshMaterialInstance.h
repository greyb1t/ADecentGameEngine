#pragma once

#include "MaterialInstance.h"

namespace Engine
{
	class MeshMaterialInstance : public MaterialInstance
	{
	public:

	protected:
		void ReflectMaterial(Reflector& aReflector) override;
	};
}