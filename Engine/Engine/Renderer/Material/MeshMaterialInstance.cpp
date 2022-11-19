#include "pch.h"
#include "MeshMaterialInstance.h"
#include "Engine\Reflection\Reflector.h"
#include "Engine\ResourceManagement\Resources\MaterialResource.h"

void Engine::MeshMaterialInstance::ReflectMaterial(Reflector& aReflector)
{
	// We reflect on the material first because the MaterialInstance::Reflect()
	// is dependant on the fact that there is an material
	if ((aReflector.Reflect(myMeshMaterial, MaterialType::Mesh, "Material") & ReflectorResult_Changed) != 0)
	{
		// NOTE(filip): Set default values before actually setting the overriden values
		// that were changed for this reflection below
		if (myMeshMaterial && myMeshMaterial->IsValid())
		{
			SetDefaultValues(*myMeshMaterial->Get());
		}
	}
}
