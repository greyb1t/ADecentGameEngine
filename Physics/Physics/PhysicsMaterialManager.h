#pragma once
#include "PhysicsEnums.h"

class PhysicsMaterialManager
{
public:
	// TODO: Make this JSONified?
	static void RegisterMaterial(ePhysicsMaterial material, float staticFriction, float dynamicFriction, float restitution);
private:
	friend class Shape; 
	static physx::PxMaterial* GetMaterial(ePhysicsMaterial material);

	static std::unordered_map<ePhysicsMaterial, physx::PxMaterial*> ourMaterials;
};

