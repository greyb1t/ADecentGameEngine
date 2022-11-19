#include "pch.h"
#include "CookingManager.h"

#include <fstream>

#include "Physics.h"
#include "Shape.h"
#include "Common/FileIO.h"

CookingManager::CookingManager()
{
	auto px = Physics::GetInstance();
	auto cookingParams = physx::PxCookingParams(px.myPxTolerancesScale);
	cookingParams.meshPreprocessParams |= physx::PxMeshPreprocessingFlag::eWELD_VERTICES;
	cookingParams.meshWeldTolerance = 10.f;

	myCooking = PxCreateCooking(
		PX_PHYSICS_VERSION, *px.myPxFoundation, cookingParams);
	assert(myCooking && "FAILED TO INIT COOKING");
}

CookingManager& CookingManager::Instance()
{
	static CookingManager instance;
	return instance;
}

bool CookingManager::CreateTriangleMesh(const std::string& aName, const std::vector<CU::Vector3f>& someVerts,
	const std::vector<unsigned>& someIndices)
{

	std::string path = "Assets/Physics/TriangleMesh/" + aName;
	path.append(".px");
	return CreateTriangleMeshPath(path, someVerts, someIndices);
}

bool CookingManager::CreateTriangleMeshPath(const std::string& aPath, const std::vector<CU::Vector3f>& someVerts, const std::vector<unsigned>& someIndices)
{
	physx::PxTriangleMeshDesc meshDesc;
	meshDesc.points.count = someVerts.size();
	meshDesc.points.stride = sizeof(physx::PxVec3);
	meshDesc.points.data = &someVerts[0];

	meshDesc.triangles.count = someIndices.size() / 3;
	meshDesc.triangles.stride = 3 * sizeof(unsigned int);
	meshDesc.triangles.data = &someIndices[0];

	physx::PxDefaultMemoryOutputStream buf;
	physx::PxTriangleMeshCookingResult::Enum result;
	if (!CookingManager::Instance().CookTriangleMesh(meshDesc, buf, &result))
	{
		if (result == physx::PxTriangleMeshCookingResult::eLARGE_TRIANGLE)
		{
			std::cout << "WARNING WARNING WARNING WARNINGPhysxShape " << aPath << ": TOO LARGE TRIANGLES WHEN COOKING" << std::endl;
		}
		return false;
	}

	if (result == physx::PxTriangleMeshCookingResult::eLARGE_TRIANGLE)
	{
		std::cout << "WARNING WARNING WARNING WARNINGPhysxShape " << aPath << ": TOO LARGE TRIANGLES WHEN COOKING" << std::endl;
	}

	if (result == physx::PxTriangleMeshCookingResult::eFAILURE)
	{
		std::cout << "Cooking failed" << std::endl;
	}

	auto bufferSize = buf.getSize();
	auto buffer = buf.getData();

	FileIO::RemoveReadOnly(aPath);

	std::ofstream file;
	file.open(aPath, std::ios::out | std::ios::trunc | std::ios::binary);
	file.write(reinterpret_cast<char*>(buffer), bufferSize);
	file.close();

	return true;
}

bool CookingManager::CreateConvex(const std::string& aName, const std::vector<CU::Vector3f>& someVerts)
{
	std::string path = "Assets/Physics/Convex/" + aName;
	path.append(".px");
	return CreateConvexPath(path, someVerts);
}

bool CookingManager::CreateConvexPath(const std::string& aPath, const std::vector<CU::Vector3f>& someVerts)
{
	physx::PxConvexMeshDesc convexDesc;
	convexDesc.points.count = someVerts.size();
	convexDesc.points.stride = sizeof(CU::Vector3f);
	convexDesc.points.data = &someVerts[0];
	convexDesc.flags = physx::PxConvexFlag::eCOMPUTE_CONVEX;

	assert(someVerts.size() <= 256 && "Too many vertices in convex! Max 256");

	physx::PxDefaultMemoryOutputStream buf;
	if (!CookingManager::Instance().CookConvexMesh(convexDesc, buf))
	{
		return false;
	}

	auto bufferSize = buf.getSize();
	auto buffer = buf.getData();

	std::ofstream file;

	file.open(aPath, std::ios::out | std::ios::trunc);
	file.write(reinterpret_cast<char*>(buffer), bufferSize);
	file.close();

	return true;
}

physx::PxConvexMesh* CookingManager::GetConvex(const std::string& aName)
{
	std::string path = "Assets/Physics/Convex/" + aName;
	path.append(".px");

	std::ifstream file(path);
	if (!file.is_open())
	{
		std::cout << "________________\n\nERROR: Couldn't open file PX Cooked Convex: " << path << std::endl;
		return nullptr;
	}

	file.seekg(0, std::ios::end);
	const int size = file.tellg();
	file.seekg(0, std::ios::beg);

	char* buffer = DBG_NEW char[size];
	file.read(buffer, size);


	file.close();

	physx::PxDefaultMemoryInputData input(reinterpret_cast<physx::PxU8*>(buffer), size);
	physx::PxConvexMesh* convexMesh = Physics::GetInstance().GetPhysics()->createConvexMesh(input);
	delete[] buffer;
	return convexMesh;
}

physx::PxTriangleMesh* CookingManager::GetTriangleMesh(const std::string& aName)
{
	std::string path = "Assets/Physics/TriangleMesh/" + aName;
	path.append(".px");

	return GetTriangleMeshPath(path);
}

physx::PxTriangleMesh* CookingManager::GetTriangleMeshPath(const std::string& aPath)
{
	std::ifstream file(aPath, std::ios::binary);
	if (!file.is_open())
	{
		std::cout << "________________\n\nERROR: Couldn't open file PX Cooked Triangle Mesh: " << aPath << std::endl;
		return nullptr;
	}

	file.seekg(0, std::ios::end);
	const int size = file.tellg();
	file.seekg(0, std::ios::beg);
	char* buffer = DBG_NEW char[size];
	file.read(buffer, size);

	file.close();

	physx::PxDefaultMemoryInputData input(reinterpret_cast<physx::PxU8*>(buffer), size);
	physx::PxTriangleMesh* triangleMesh = Physics::GetInstance().GetPhysics()->createTriangleMesh(input);
	delete[] buffer;
	return triangleMesh;
}

bool CookingManager::CookConvexMesh(const physx::PxConvexMeshDesc& desc, physx::PxOutputStream& stream,
                                    physx::PxConvexMeshCookingResult::Enum* condition) const
{
	return myCooking->cookConvexMesh(desc, stream, condition);
}

bool CookingManager::CookTriangleMesh(const physx::PxTriangleMeshDesc& desc, physx::PxOutputStream& stream,
                                      physx::PxTriangleMeshCookingResult::Enum* condition) const
{
	return myCooking->cookTriangleMesh(desc, stream, condition);
}
