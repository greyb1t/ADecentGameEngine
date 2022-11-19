#pragma once

#include "Shape.h"
#include "Types.h"

class PhysicsScene;

class RigidStatic
{
public:
	RigidStatic() = default;
	RigidStatic(
		const CU::Vector3f&		aPosition,
		const CU::Quaternion&	aRotation = CU::Vector3f {0, 0, 0},
		Layer					aFilterGroup = 0,
		LayerMask				aPhysicalFilterMask = 0,
		LayerMask				aDetectionMask = 0);

	virtual ~RigidStatic();


	virtual void			Attach(Shape aShape);

	void					SetPosition(const CU::Vector3f&);
	void					SetTransform(const CU::Vector3f&, const CU::Quaternion&);
	void					Move(float x, float y, float z);

	void					SetTrigger(bool aValue);
	bool					IsTrigger() const;
	
	void					SetupFiltering(Layer aFilterGroup, LayerMask aPhysicalFilterMask, LayerMask aDetectionLayerMask);
	void					SetCollisionListener(bool aValue);
	bool					IsCollisionListener() const;

	void					ConnectUUID(PUUID aID);
	PUUID					GetUUID() const;

	physx::PxRigidStatic*	GetActor() const;
	std::vector<Shape>&		GetShapes() { return myShapes; }

protected:
	bool					Init(
		const CU::Vector3f& aPosition = {0, 0, 0},
		const CU::Quaternion& aRotation = CU::Vector3f {0, 0, 0},
		Layer aFilterGroup = 0,
		LayerMask aPhysicalFilterMask = 0,
		LayerMask aDetectionMask = 0);

protected:
	friend class PhysicsScene;
	PUUID					myUUID = -1;

	physx::PxRigidStatic*	myRigidStatic = nullptr;
	std::vector<Shape>		myShapes;

	bool					myCollisionListener = false;
	bool					myIsTrigger = false;

	unsigned int			myFilterGroup =				(1 << 0);
	unsigned int			myPhysicalFilterTargets =	(1 << 0);
	unsigned int			myDetectionFilterTargets =	(1 << 0);

	PhysicsScene*			myScene = nullptr;

	// Used to make sure no memory is leaking.
	PUUID* myUserPUUID = nullptr;
};

