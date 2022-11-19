#pragma once

#include "Types.h"
#include "Shape.h"

namespace physx
{
	class PxRigidDynamic;
	class PxShape;
}

class PhysicsScene;

enum class eRigidBodyType
{
	DEFAULT,
	KINEMATIC,
	TRIGGER
};

class RigidBody
{
public:

public:
	RigidBody(
		eRigidBodyType			aType = eRigidBodyType::DEFAULT, 
		const CU::Vector3f&		aPosition = { 0,0,0 }, 
		const CU::Quaternion&	aRotation = CU::Vector3f{ 0,0,0 }, 
		Layer					aFilterGroup = 0, 
		LayerMask				aPhysicalFilterMask = 0, 
		LayerMask				aTriggerFilterMask = 0);

	virtual ~RigidBody();

	virtual void		SetType(eRigidBodyType aType);

	virtual void		Attach(Shape aShape);
	virtual void		Detach(const Shape& aShape);


	void				SetPosition(const CU::Vector3f& aPosition);
	void				SetRotation(const CU::Vector3f& aRotation);
	void				SetRotation(const CU::Quaternion& aRotation);
	void				Move(const CU::Vector3f& aMovement, const CU::Quaternion& aQuaternion);
	void				Move(const CU::Vector3f& aMovement);



	//	VELOCITY
	void				SetVelocity(const CU::Vector3f&);
	void				SetAngularVelocity(const CU::Vector3f&);

	CU::Vector3f		GetVelocity() const;

	void				AddForce(const CU::Vector3f& aForce, eForceMode::Enum aMode);
	void				AddForceAtPosition(const CU::Vector3f& aForce, const CU::Vector3f& aPosition, eForceMode::Enum aMode);
	void				AddForceAtLocalPosition(const CU::Vector3f& aForce, const CU::Vector3f& aLocalPosition, eForceMode::Enum aMode);
	//	~VELOCITY

	void				SetRotationConstraints(bool x, bool y, bool z);

	void				SetMass(float aMass);

	void				SetCollisionListener(bool aValue);
	bool				IsCollisionListener() const;

	void				ConnectUUID(PUUID aID);

	eRigidBodyType				GetType() const;
	PUUID						GetUUID() const;

	physx::PxRigidDynamic*		GetActor() const;
	const std::vector<Shape>&	GetShapes() const { return myShapes; }
protected:
	void				SetupFiltering(Layer aFilterGroup, LayerMask aPhysicalFilterMask, LayerMask aDetectionFilterMask);

	void				Init(
		eRigidBodyType aType				= eRigidBodyType::DEFAULT,
		const CU::Vector3f& aPosition		= CU::Vector3f{ 0,0,0 },
		const CU::Quaternion& aRotation		= CU::Quaternion(CU::Vector3f{ 0,0,0 }),
		Layer aFilterGroup					= 0,
		LayerMask aPhysicalFilterMask		= 0,
		LayerMask aDetectionFilterMask		= 0);

	void				UpdateMass();
protected:
	friend class PhysicsScene;
	PUUID				myUUID = -1;

	physx::PxRigidDynamic* myRigidBody = nullptr;
	std::vector<Shape>	myShapes;

	float				myMass = 1.f;
	bool				myCollisionListener = false;
	unsigned int		myFilterGroup = (1 << 0);
	unsigned int		myPhysicalFilterTargets = (1 << 0);
	unsigned int		myDetectionFilterTargets = (1 << 0);

	eRigidBodyType		myType = eRigidBodyType::DEFAULT;

	PhysicsScene*		myScene = nullptr;

	// Used to make sure no memory is leaked.
	PUUID* myUserPUUID = nullptr;
};

