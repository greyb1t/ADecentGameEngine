#include "pch.h"
#include "PhysicsMaterialManager.h"
#include "Physics.h"

std::unordered_map<ePhysicsMaterial, physx::PxMaterial*> PhysicsMaterialManager::ourMaterials{ 0 };

void PhysicsMaterialManager::RegisterMaterial(ePhysicsMaterial material, float staticFriction, float dynamicFriction, float restitution)
{
	auto pxMaterial = Physics::GetInstance().GetPhysics()->createMaterial(staticFriction, dynamicFriction, restitution);
	ourMaterials[material] = pxMaterial;
}

physx::PxMaterial* PhysicsMaterialManager::GetMaterial(ePhysicsMaterial material)
{
	assert(ourMaterials.find(material) != ourMaterials.end() && "WARNING: Trying to use unregistered material.");

	return ourMaterials[material];
}
