#pragma once
#include "Shape.h"

class CookingManager
{
public:
	CookingManager();
	static CookingManager& Instance();


	bool					CreateTriangleMesh(const std::string& aName, const std::vector<CU::Vector3f>& someVerts, const std::vector<unsigned int>& someIndices);
	bool					CreateTriangleMeshPath(const std::string& aPath, const std::vector<CU::Vector3f>& someVerts, const std::vector<unsigned int>& someIndices);

	bool					CreateConvex(const std::string& aName, const std::vector<CU::Vector3f>& someVerts);
	bool					CreateConvexPath(const std::string& aPath, const std::vector<CU::Vector3f>& someVerts);

	// Creates geometry buffer.
	bool					CookConvexMesh(const physx::PxConvexMeshDesc& desc, physx::PxOutputStream& stream, physx::PxConvexMeshCookingResult::Enum* condition = NULL) const;
	// Creates geometry buffer.
	bool					CookTriangleMesh(const physx::PxTriangleMeshDesc& desc, physx::PxOutputStream& stream, physx::PxTriangleMeshCookingResult::Enum* condition = NULL) const;
private:
	friend class Shape;
	physx::PxConvexMesh*	GetConvex(const std::string& aName);
	physx::PxTriangleMesh*	GetTriangleMesh(const std::string& aName);
	physx::PxTriangleMesh*	GetTriangleMeshPath(const std::string& aPath);



	physx::PxCooking* myCooking = nullptr;
};

