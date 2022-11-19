#pragma once
#include "Shape.h"
#include "PhysicsEnums.h"

namespace physx
{
	class PxShape;
}

class  Shape
{
public:
	enum eGeometry {
		eBOX,
		eSPHERE,
		eCAPSULE,
		eCONVEX,
		eTRIANGLE_MESH,
	};

	// TO BE REMOVED
	Shape() = default;
	Shape(physx::PxShape* shape) { myShape = shape; }

	bool IsValid() const;

	void						SetLocalPosition(const CU::Vector3f& aPosition);
	void						SetLocalQuaternion(const CU::Quaternion& aRotation);
	CU::Vector3f				GetLocalPosition() const;
	CU::Quaternion GetLocalQuaternion() const;

	void						SetName(const std::string& aName);
	std::string					GetName() const;

	void						SetData(void* aData) const;
	void*						GetData() const;

	void						SetupFiltering(unsigned int aFilterGroup, unsigned int aPhysicalFilterMask, unsigned int aDetectionFilterMask); 
	void						SetTrigger(bool);
	eGeometry					GetType() const;
	
	float						GetRadius() const;
	float						GetHeight() const;
	CU::Vector3f				GetBoxHalfSize() const;

	std::vector<CU::Vector3f>	GetConvexVertices() const;
	std::vector<CU::Vector3f>	GetConvexIndices() const;

	static Shape			Box(const CU::Vector3f& size, ePhysicsMaterial material = ePhysicsMaterial::Default, bool isExclusive = true);
	static Shape			Sphere(float radius, ePhysicsMaterial material = ePhysicsMaterial::Default, bool isExclusive = true);
	static Shape			Capsule(float halfHeight, float radius, ePhysicsMaterial material = ePhysicsMaterial::Default, bool isExclusive = true);
	static Shape			Convex(const std::string& aName, ePhysicsMaterial material = ePhysicsMaterial::Default, bool isExclusive = true);
	static Shape			Convex(const std::vector<CU::Vector3f>& someVerts, ePhysicsMaterial material = ePhysicsMaterial::Default, bool isExclusive = true);
	static Shape			TriangleMesh(const std::string& aName, ePhysicsMaterial material = ePhysicsMaterial::Default, bool isExclusive = true);
	static Shape			TriangleMeshPath(const std::string& aPath, ePhysicsMaterial material = ePhysicsMaterial::Default, bool isExclusive = true);

	static Shape			TriangleMesh(const std::vector<CU::Vector3f>& someVerts, const std::vector<unsigned>& someIndices, ePhysicsMaterial material = ePhysicsMaterial::Default, bool isExclusive = true);
	static Shape			Copy(Shape& aShape);
protected:
	friend class RigidBody;
	friend class RigidStatic;

	bool				myIsExclusive = true;

	physx::PxShape*		myShape = nullptr;
	eGeometry			myGeometry = eGeometry::eBOX;
	ePhysicsMaterial	myMaterial = ePhysicsMaterial::Default;
};

