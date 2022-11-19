#include "pch.h"
#include "CharacterController.h"
#include "PhysicsScene.h"
#include "Physics.h"
#include "CCTHitReporter.h"

CharacterController::CharacterController(PhysicsScene* aScene)
	: myScene(aScene)
{
	
}

CharacterController::~CharacterController()
{
	if (myScene)
	{
		if (myController)
		{
			myController->release();
		}
	}

	if (myHitReporter)
	{
		delete myHitReporter;
		myHitReporter = nullptr;
	}

	if (myMoveFilter)
	{
		delete myMoveFilter;
		myMoveFilter = nullptr;
	}

	if (myUserPUUID)
	{
		delete myUserPUUID;
		myUserPUUID = nullptr;
	}

	myScene = nullptr;
	myController = nullptr;

}

void CharacterController::Init(const CU::Vector3f& aPosition, float aHeight, float aRadius)
{
	// This should all be possible to be changed in init later

	constexpr float Deg2Rad = 3.14159265359f / 180.f;
	physx::PxCapsuleControllerDesc desc;
	desc.setToDefault();
	desc.contactOffset = 1.f;
	desc.stepOffset = .1f;
	desc.height = aHeight;
	desc.radius = aRadius;
	desc.position = { aPosition.x, aPosition.y, aPosition.z };
	desc.upDirection = { 0,1,0 };
	desc.slopeLimit = 10.f * Deg2Rad;
	desc.material = Physics::GetInstance().GetMaterial();

	myHitReporter = DBG_NEW CCTHitReporter();
	desc.behaviorCallback = myHitReporter;
	desc.reportCallback = myHitReporter;
	desc.scaleCoeff = .9f;

	if (desc.isValid()) {
		int a = 0;
	}
	else {
		std::cout << "CCT Not valid" << std::endl;
	}

	myController = myScene->GetControllerManager().createController(desc);
	assert(myController && "Failed to create CharacterController!");

	myHeight = aHeight;
	myRadius = aRadius;
}

void CharacterController::SetMovementSettings(float aMinDistance)
{
	myMinDist = aMinDistance;
}

void CharacterController::SetPosition(const CU::Vector3f& aPosition)
{
	myController->setPosition({ aPosition.x, aPosition.y, aPosition.z });
}

void CharacterController::Move(const CU::Vector3f& aMovement, float aDeltaTime)
{
	//myMoveFilter->ClearHits();
	myController->move({ aMovement.x, aMovement.y, aMovement.z }, myMinDist, aDeltaTime, myControllerFilters);
	//auto& hits = myMoveFilter->GetHits();
	//std::cout << hits.size() << std::endl;
}

void CharacterController::Resize(float aHeight)
{
	myHeight = aHeight;
	myController->resize(aHeight);
}

bool CharacterController::IsGrounded()
{
	physx::PxControllerState state;
	myController->getState(state);

	return state.touchedActor;
} 

void CharacterController::SetupFiltering(Layer aFilterGroup, LayerMask aPhysicalFilterMask, LayerMask aTriggerFilterMask)
{
	myFilterGroup = aFilterGroup;
	myPhysicalFilterTargets = aPhysicalFilterMask;
	myDetectionFilterTargets = aTriggerFilterMask;

	physx::PxFilterData filterData;
	filterData.word0 = aFilterGroup;		// Actors ID
	filterData.word1 = aPhysicalFilterMask; // Mask for which ID's will activate physical pushback
	filterData.word2 = aTriggerFilterMask;	// Mask for which ID's will trigger collision detection
	// Not used yet
	//filterData.word3 = aPhysicalFilterMask;	// Mask for which ID's will activate physical pushback (CCT SPECIFIC)

	auto* actor = myController->getActor();
	physx::PxShape* shape;  
	actor->getShapes(&shape, 1);
	shape->setSimulationFilterData(filterData);
	shape->setQueryFilterData(filterData);

	//myControllerFilters.mFilterFlags = physx::PxQueryFlag::eSTATIC | physx::PxQueryFlag::eDYNAMIC | physx::PxQueryFlag::ePREFILTER | physx::PxQueryFlag::ePOSTFILTER;
	if (myControllerFilters.mFilterData)
		delete myControllerFilters.mFilterData;
	//if (myControllerFilters.mFilterCallback)
	//	delete myControllerFilters.mFilterCallback;

	myControllerFilters.mFilterData = DBG_NEW physx::PxFilterData(myFilterGroup, myPhysicalFilterTargets, myDetectionFilterTargets, myPhysicalFilterTargets);

	if (!myMoveFilter)
	{
		myMoveFilter = DBG_NEW FilterCCTMove();
	}
	myControllerFilters.mFilterCallback = myMoveFilter;
	myMoveFilter->SetMask(aPhysicalFilterMask);
}

void CharacterController::SetCollisionListener(bool aValue)
{
	myCollisionListener = aValue;

	if (myScene)
		myScene->GetCollisionRegistry().RegisterUUID(myUUID, aValue);
}

bool CharacterController::IsCollisionListener() const
{
	return myCollisionListener;
}

void CharacterController::ConnectUUID(PUUID aID)
{
	myUUID = aID;
	// TODO: FIX DELETE FO PUUID
	myUserPUUID = DBG_NEW PUUID(aID);
	myController->setUserData(myUserPUUID);
	myController->getActor()->userData = myUserPUUID;

	if (myScene)
		myScene->GetCollisionRegistry().RegisterUUID(aID);
}

PUUID CharacterController::GetUUID() const
{
	return myUUID;
}

float CharacterController::GetHeight() const
{
	return myHeight;
}

float CharacterController::GetRadius() const
{
	return myRadius;
}

physx::PxRigidDynamic* CharacterController::GetActor() const
{
	return myController->getActor();
}
