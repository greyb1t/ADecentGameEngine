#include "pch.h"
#include "PhysicsScene.h"

#include <cassert>

#include "PhysicsListener.h"
#include "Physics.h"
#include "RayCastHitProcessor.h"
#include "RigidBody.h"
#include "RigidStatic.h"
#include <PxActor.h>

#include "OverlapHitInternal.h"
#include "Filters/PhysXQueryFilter.h"

bool PhysicsScene::Init()
{
	myScene = Physics::GetInstance().CreateScene();
	myListener = DBG_NEW PhysicsListener();
	myControllerManager = PxCreateControllerManager(*myScene);

	myScene->setSimulationEventCallback(myListener);

	physx::PxPvdSceneClient* pvdSceneClient = myScene->getScenePvdClient();
	if (pvdSceneClient)
	{
		pvdSceneClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
		pvdSceneClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
		pvdSceneClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);

		myScene->setVisualizationParameter(physx::PxVisualizationParameter::eJOINT_LOCAL_FRAMES, 1.0f);
		myScene->setVisualizationParameter(physx::PxVisualizationParameter::eJOINT_LIMITS, 1.0f);

		if (!PxInitExtensions(*Physics::GetInstance().GetPhysics(), Physics::GetInstance().GetPVD()))
			assert(false && "Well it failed, call me maybe baby! - Viktor");
	}
	return true;
}

void PhysicsScene::SimulatePhysics(float dt)
{
	myListener->ResetCollisions();

	dt = fminf(dt, 0.0333f);
	myScene->simulate(dt);
	//myScene->fetchResults(true);
	//
	//GenerateCollisions();
}

void PhysicsScene::FetchPhysics(float dt)
{
	myScene->fetchResults(true);
	GenerateCollisions();
}

bool PhysicsScene::RayCast(const CU::Vector3f& aOrigin, const CU::Vector3f& aDirection, float aRange,
	LayerMask aLayerMask, RayCastHitInternal& outHit)
{
	constexpr unsigned int bufferSize = 32;
	physx::PxRaycastHit hits[bufferSize];
	RayCastHitProcessor raycastProcessor(hits, bufferSize);

	physx::PxQueryFilterData fd;
	fd.flags |= physx::PxQueryFlag::eDYNAMIC | physx::PxQueryFlag::eSTATIC | physx::PxQueryFlag::ePREFILTER;

	PhysXQueryFilter callback(aLayerMask);

	const physx::PxHitFlags hitFlags(physx::PxHitFlag::eDEFAULT);

	bool result = myScene->raycast(
		{ aOrigin.x, aOrigin.y, aOrigin.z }, { aDirection.x, aDirection.y, aDirection.z }, aRange,
		raycastProcessor, hitFlags, fd, &callback);

	outHit = raycastProcessor.myClosestHit;
	outHit.Origin = aOrigin;
	return result;
}

bool PhysicsScene::RayCastAll(const CU::Vector3f& aOrigin, const CU::Vector3f& aDirection, float aRange, LayerMask aLayerMask, std::vector<RayCastHitInternal>& outHit)
{
	constexpr unsigned int bufferSize = 32;
	physx::PxRaycastHit hits[bufferSize];
	RayCastHitProcessor raycastProcessor(hits, bufferSize);
	
	physx::PxQueryFilterData fd;
	fd.flags |= physx::PxQueryFlag::eDYNAMIC | physx::PxQueryFlag::eSTATIC | physx::PxQueryFlag::ePREFILTER;

	PhysXQueryFilter callback(aLayerMask);
	
	const physx::PxHitFlags hitFlags(physx::PxHitFlag::eDEFAULT);

	bool result = myScene->raycast(
		{ aOrigin.x, aOrigin.y, aOrigin.z }, { aDirection.x, aDirection.y, aDirection.z }, aRange, 
		raycastProcessor, hitFlags, fd, &callback);

	outHit = raycastProcessor.myRayCastHits;
	return result;
}

bool PhysicsScene::GeometryCastAll(const physx::PxGeometry& aGeometry, const CU::Vector3f& aPosition, const CU::Quaternion& aQuaternion, LayerMask aLayerMask, std::vector<OverlapHitInternal>& outHits)
{
	physx::PxTransform pose(physx::PxVec3(physx::PxReal(aPosition.x), physx::PxReal(aPosition.y), physx::PxReal(aPosition.z)), physx::PxQuat(aQuaternion.myVector.x, aQuaternion.myVector.y, aQuaternion.myVector.z, aQuaternion.myW));

	// NOTE: YOU CAN ONLY CAST HIT A TOTAL OF 64, aka one stack, get em diamonds!
	constexpr physx::PxU32 bufferSize = 64;
	physx::PxOverlapHit hits[bufferSize];
	physx::PxOverlapBuffer buf(hits, bufferSize);


	// physx::PxQueryFlag::ePREFILTER makes the callback preFiltering run and makes layer masking work (hopefully)
	physx::PxQueryFilterData fd;
	fd.flags |= physx::PxQueryFlag::eDYNAMIC | physx::PxQueryFlag::eSTATIC | physx::PxQueryFlag::ePREFILTER;

	PhysXQueryFilter callback(aLayerMask);

	if (myScene->overlap(aGeometry, pose, buf, fd, &callback))
	{
		auto size = buf.getNbTouches();
		for (int i = 0; i < size; i++)
		{
			auto& hit = hits[i];
			OverlapHitInternal oh{ hit.actor };
			outHits.emplace_back(oh);
		}
		return true;
	}

	return false;
}

bool PhysicsScene::BoxCastAll(const CU::Vector3f& aPosition, const CU::Quaternion& aQuaternion, const CU::Vector3f& aHalfSize, LayerMask aLayerMask,
	std::vector<OverlapHitInternal>& outHits)
{
	// TODO: Add debug drawing of overlap cast
	physx::PxTransform pose(physx::PxVec3(physx::PxReal(aPosition.x), physx::PxReal(aPosition.y), physx::PxReal(aPosition.z)), physx::PxQuat(aQuaternion.myVector.x, aQuaternion.myVector.y, aQuaternion.myVector.z, aQuaternion.myW));

	const auto box = physx::PxBoxGeometry(aHalfSize.x, aHalfSize.y, aHalfSize.z);

	// NOTE: YOU CAN ONLY CAST HIT A TOTAL OF 64, aka one stack, get em diamonds!
	constexpr physx::PxU32 bufferSize = 64;
	physx::PxOverlapHit hits[bufferSize];
	physx::PxOverlapBuffer buf(hits, bufferSize);


	// physx::PxQueryFlag::ePREFILTER makes the callback preFiltering run and makes layer masking work (hopefully)
	physx::PxQueryFilterData fd;
	fd.flags |= physx::PxQueryFlag::eDYNAMIC | physx::PxQueryFlag::eSTATIC | physx::PxQueryFlag::ePREFILTER;

	PhysXQueryFilter callback(aLayerMask);

	if (myScene->overlap(box, pose, buf, fd, &callback))
	{
		auto size = buf.getNbTouches();
		for (int i = 0; i < size; i++)
		{
			auto& hit = hits[i];
			OverlapHitInternal oh{ hit.actor };
			outHits.emplace_back(oh);
		}
		return true;
	}

	return false;
}

bool PhysicsScene::SphereCastAll(const CU::Vector3f& aPosition, float aRadius, LayerMask aLayerMask, std::vector<OverlapHitInternal>& outHits)
{
	// TODO: Add debug drawing of overlap cast
	physx::PxTransform pose(physx::PxVec3(physx::PxReal(aPosition.x), physx::PxReal(aPosition.y), physx::PxReal(aPosition.z)));

	const auto sphere = physx::PxSphereGeometry(aRadius);

	// NOTE: YOU CAN ONLY CAST HIT A TOTAL OF 64, aka one stack, get em diamonds!
	constexpr physx::PxU32 bufferSize = 64;
	physx::PxOverlapHit hits[bufferSize];
	physx::PxOverlapBuffer buf(hits, bufferSize);


	// physx::PxQueryFlag::ePREFILTER makes the callback preFiltering run and makes layer masking work (hopefully)
	physx::PxQueryFilterData fd;
	fd.flags |= physx::PxQueryFlag::eDYNAMIC | physx::PxQueryFlag::eSTATIC | physx::PxQueryFlag::ePREFILTER;

	PhysXQueryFilter callback(aLayerMask);

	if (myScene->overlap(sphere, pose, buf, fd, &callback))
	{
		auto size = buf.getNbTouches();
		for (int i = 0; i < size; i++)
		{
			auto& hit = hits[i];
			OverlapHitInternal oh{ hit.actor };
			outHits.emplace_back(oh);
		}
		return true;
	}

	return false;
}

void PhysicsScene::SetGravity(const CU::Vector3f& aGravity)
{
	myScene->setGravity( physx::PxVec3{ aGravity.x,aGravity.y,aGravity.z });
}

void PhysicsScene::AddActor(RigidBody* aRigidBody)
{
	aRigidBody->myScene = this;
	if (aRigidBody->GetUUID() != -1) 
	{
		myRegister.RegisterUUID(aRigidBody->GetUUID(), aRigidBody->myCollisionListener);
	}

	myScene->addActor(*aRigidBody->GetActor());
}

void PhysicsScene::AddActor(RigidStatic* aRigidStatic)
{
	aRigidStatic->myScene = this;
	if (aRigidStatic->GetUUID() != -1)
	{
		myRegister.RegisterUUID(aRigidStatic->GetUUID(), aRigidStatic->myCollisionListener);
	}

	myScene->addActor(*aRigidStatic->GetActor());
}

void PhysicsScene::RemoveActor(RigidBody* aRigidBody)
{
	myScene->removeActor(*aRigidBody->GetActor());
	if (aRigidBody->GetUUID() != -1)
		myRegister.UnregisterUUID(aRigidBody->GetUUID());

	aRigidBody->myScene = nullptr;
}

void PhysicsScene::RemoveActor(RigidStatic* aRigidStatic)
{
	myScene->removeActor(*aRigidStatic->GetActor());
	if (aRigidStatic->GetUUID() != -1)
		myRegister.UnregisterUUID(aRigidStatic->GetUUID());

	aRigidStatic->myScene = nullptr;
}

physx::PxScene* PhysicsScene::GetScene() const
{
	return myScene;
}

physx::PxControllerManager& PhysicsScene::GetControllerManager() const
{
	return *myControllerManager;
}

CollisionRegister& PhysicsScene::GetCollisionRegistry()
{
	return myRegister;
}

const std::vector<CollisionData>& PhysicsScene::GetCollisions() const
{
	return myListener->GetCollisions();
}

void PhysicsScene::GenerateCollisions()
{
	myRegister.UpdateStates();
	auto& collisionsData = GetCollisions();

	for (auto& data : collisionsData)
	{
		myRegister.RegisterCollision(data.a.uuid, data);
		myRegister.RegisterCollision(data.b.uuid, data);
	}
}
