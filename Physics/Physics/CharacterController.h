#pragma once
#include "Types.h"
#include "Filters/FilterCCTMove.h"

class PhysicsScene;
class CCTHitReporter;

class CharacterController
{
public:
	CharacterController() = default;
	CharacterController(PhysicsScene* aScene);
	~CharacterController();

	virtual void		Init(const CU::Vector3f& aPosition, float aHeight, float aRadius);
	void				SetMovementSettings(float aMinDistance);

	void				SetPosition(const CU::Vector3f& aPosition);
	void				Move(const CU::Vector3f& aMovement, float aDeltaTime);
	void				Resize(float aHeight);

	virtual bool		IsGrounded();

	void				SetCollisionListener(bool aValue);
	bool				IsCollisionListener() const;


	void				ConnectUUID(PUUID aID);
	PUUID				GetUUID() const;
	float				GetHeight() const;
	float				GetRadius() const;

	physx::PxRigidDynamic* GetActor() const;

protected:
	void				SetupFiltering(Layer aFilterGroup, LayerMask aPhysicalFilterMask, LayerMask aTriggerFilterMask);
protected:
	friend class PhysicsScene;

	PhysicsScene* myScene = nullptr;
	physx::PxController* myController = nullptr;

	PUUID				myUUID = -1;
	bool				myCollisionListener = false;

	unsigned int		myFilterGroup = 0;
	unsigned int		myPhysicalFilterTargets = 0;
	unsigned int		myDetectionFilterTargets = 0;
	
	float myRadius = 0;
	float myHeight = 0;

	/// Movement Settings
	float myMinDist = .1f;
	physx::PxControllerFilters myControllerFilters = 0;
	CCTHitReporter* myHitReporter = nullptr;
	FilterCCTMove* myMoveFilter = nullptr;
	///

	PUUID* myUserPUUID = nullptr;
};

