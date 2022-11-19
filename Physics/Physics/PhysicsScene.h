#pragma once

#include "RayCastHitInternal.h"
#include "Types.h"
#include "CharacterController.h"
#include "CollisionRegister.h"

class PhysXQueryFilter;
struct OverlapHitInternal;

namespace physx
{
	class PxScene;
	class PxActor;
}

class RigidStatic;
class PhysicsListener;
class RigidBody;
class PhysicsControllerBehaviourCallback;

class PhysicsScene
{
public:
	bool	Init();

	void	SimulatePhysics(float dt);
	void	FetchPhysics(float dt);


	void	SetGravity(const CU::Vector3f& aGravity);

	void	AddActor(RigidBody* aRigidBody);
	void	RemoveActor(RigidBody* aRigidBody);

	void	AddActor(RigidStatic* aRigidStatic);
	void	RemoveActor(RigidStatic* aRigidStatic);
	
	physx::PxScene*						GetScene() const;
	physx::PxControllerManager&			GetControllerManager() const;

	CollisionRegister&					GetCollisionRegistry();

	const std::vector<CollisionData>&	GetCollisions() const;

protected:
	bool	RayCast(const CU::Vector3f& aOrigin, const CU::Vector3f& aDirection, float aRange, unsigned int aLayerMask, RayCastHitInternal& outHit);
	bool	RayCastAll(const CU::Vector3f& aOrigin, const CU::Vector3f& aDirection, float aRange, unsigned int aLayerMask, std::vector<RayCastHitInternal>& outHit);

	bool	GeometryCastAll(const physx::PxGeometry& aGeometry, const CU::Vector3f& aPosition, const CU::Quaternion& aQuaternion, LayerMask aLayerMask, std::vector<OverlapHitInternal>& outHits);
	bool	BoxCastAll(const CU::Vector3f& aPosition, const CU::Quaternion& aQuaternion, const CU::Vector3f& aHalfSize, LayerMask aLayerMask, std::vector<OverlapHitInternal>& outHits);
	bool	SphereCastAll(const CU::Vector3f& aPosition, float aRadius, LayerMask aLayerMask, std::vector<OverlapHitInternal>& outHits);
private:
	void GenerateCollisions();

	physx::PxScene*						myScene = nullptr;
	physx::PxControllerManager*			myControllerManager = nullptr;
	PhysicsControllerBehaviourCallback* myControllerBehaviorCallback = nullptr;

	PhysicsListener*					myListener = nullptr;

	CollisionRegister					myRegister;
};