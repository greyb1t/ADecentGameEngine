#pragma once
#include "Component.h"

class RigidStatic;

namespace Engine
{
	class MultiRigidStatic :
	    public Component
	{
	public:
		MultiRigidStatic(GameObject* aGameObject);
		~MultiRigidStatic();

		void Execute(eEngineOrder aOrder) override;

		void AddRigidStatic(RigidStatic* aStatic, const CU::Vector3f& aOffset = {0,0,0}, const CU::Quaternion& aRotation = CU::Quaternion{{0,0,0}});
	private:
		std::vector<RigidStatic*> myRigidStatics;
	};
}
