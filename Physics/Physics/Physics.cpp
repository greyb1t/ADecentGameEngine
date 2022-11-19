#include "pch.h"
#include "Physics.h"

#include "CookingManager.h"
#include "Filters/PhysXFilterShader.h"
#include "RigidBody.h"
#include "CookingManager.h"

Physics* Physics::ourInstance = nullptr;

bool Physics::Init()
{
	ourInstance = this;

	myPxFoundation = PxCreateFoundation(
		PX_PHYSICS_VERSION, myPxDefaultAllocatorCallback, myPxDefaultErrorCallback);
	assert(myPxFoundation && "Failed to create Physics Foundation!");
	if (!myPxFoundation)
		return false;

#ifndef _RETAIL
	InitPVD();
#endif

	myPxTolerancesScale.length
		= 100.f; // This is default size of a collider, since we have 100cm per meter we will use 100.f
	myPxTolerancesScale.speed
		= 981.f * 2.f; // A good default speed was said to be eq to gravity. MIGHT change later.

	myPhysics
		= PxCreatePhysics(PX_PHYSICS_VERSION, *myPxFoundation, myPxTolerancesScale, true, myPvd);
	assert(myPhysics && "Failed to create physics SDK!");
	if (!myPhysics)
		return false;

	myPxDefaultCpuDispatcher
		= physx::PxDefaultCpuDispatcherCreate(4); // Num of threads to use when simulating physics

	CookingManager::Instance();

	// TODO: Find out what material parameters should be like
	myPxMaterial = myPhysics->createMaterial(0.5f, .8f, 0.6f);

	return true;
}

void Physics::SetMaterial(float aStaticFriction, float aDynamicFriction, float aRestitution)
{
	myPxMaterial->setStaticFriction(aStaticFriction);
	myPxMaterial->setDynamicFriction(aDynamicFriction);
	myPxMaterial->setRestitution(aRestitution);
}

physx::PxMaterial* Physics::GetMaterial() const
{
	return myPxMaterial;
}

physx::PxScene* Physics::CreateScene()
{
	physx::PxSceneDesc sceneDesc(myPhysics->getTolerancesScale());

	sceneDesc.gravity = {0.f, -981.f * 2.f, 0.f};
	sceneDesc.cpuDispatcher = myPxDefaultCpuDispatcher;

	//sceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;
	sceneDesc.filterShader = PhysicsFilter;

	return myPhysics->createScene(sceneDesc);
}

//physx::PxShape* Physics::CreateShape(const physx::PxGeometry& aGeometry, bool isExclusive)
//{
//	return myPhysics->createShape(aGeometry, *myPxMaterial, isExclusive);
//}

physx::PxCapsuleController* Physics::CreateCharacterController()
{

	return nullptr;
}

physx::PxPhysics* Physics::GetPhysics()
{
	return myPhysics;
}

physx::PxPvd* Physics::GetPVD()
{
	return myPvd;
}

bool Physics::ConnectPVD()
{
	if (ourInstance->myPvd->isConnected())
	{
		ourInstance->myPvd->disconnect();
	}
	return ourInstance->myPvd->connect(
		*ourInstance->myPvdTransport, physx::PxPvdInstrumentationFlag::eALL);
}

Physics& Physics::GetInstance()
{
	return *ourInstance;
}
//
//physx::PxShape* Physics::CreateBox(const CU::Vector3f& aSize, bool isExclusive)
//{
//	return GetInstance().CreateShape(physx::PxBoxGeometry(aSize.x, aSize.y, aSize.z), isExclusive);
//}
//
//physx::PxShape* Physics::CreateSphere(float aRadius, bool isExclusive)
//{
//	return GetInstance().CreateShape(physx::PxSphereGeometry(aRadius), isExclusive);
//}
//
//physx::PxShape* Physics::CreateCapsule(float aRadius, float aHalfLength, bool isExclusive)
//{
//	return GetInstance().CreateShape(physx::PxCapsuleGeometry(aRadius, aHalfLength), isExclusive);
//}
//
//physx::PxShape* Physics::CreateConvex(const std::vector<CU::Vector3f>& someVerts, bool isExclusive)
//{
//	physx::PxConvexMeshDesc convexDesc;
//	convexDesc.points.count = someVerts.size();
//	convexDesc.points.stride = sizeof(CU::Vector3f);
//	convexDesc.points.data = &someVerts[0];
//	convexDesc.flags = physx::PxConvexFlag::eCOMPUTE_CONVEX;
//
//	physx::PxDefaultMemoryOutputStream buf;
//	if (!ourInstance->myCooking->cookConvexMesh(convexDesc, buf))
//		return NULL;
//
//	physx::PxDefaultMemoryInputData input(buf.getData(), buf.getSize());
//	physx::PxConvexMesh* convexMesh = ourInstance->GetPhysics()->createConvexMesh(input);
//	return ourInstance->CreateShape(physx::PxConvexMeshGeometry(convexMesh));
//}
//
//physx::PxShape* Physics::CreateHeightMap(
//	/*physx::PxRigidActor* aActor,*/ const int_fast16_t* aHeightMap, int aNumRows, int aNumColums)
//{
//	physx::PxHeightFieldSample* samples = (physx::PxHeightFieldSample*)malloc(
//		sizeof(physx::PxHeightFieldSample) * (aNumRows * aNumColums));
//
//	for (int i = 0; i < aNumRows * aNumColums; i++)
//	{
//		samples[i].height = aHeightMap[i];
//	}
//
//	physx::PxHeightFieldDesc hfDesc;
//	hfDesc.format = physx::PxHeightFieldFormat::eS16_TM;
//	hfDesc.nbColumns = aNumColums;
//	hfDesc.nbRows = aNumRows;
//	hfDesc.samples.data = samples;
//	hfDesc.samples.stride = sizeof(physx::PxHeightFieldSample);
//
//	physx::PxHeightField* heightField = GetInstance().myCooking->createHeightField(
//		hfDesc, GetInstance().myPhysics->getPhysicsInsertionCallback());
//
//	physx::PxHeightFieldGeometry hfGeom(heightField, physx::PxMeshGeometryFlags(), 1.f, 50, 50);
//
//	// IF we want to add support for different slippery/friction on ground we can add multi material support.
//	//physx::PxShape* hfShape = physx::PxRigidActorExt::createExclusiveShape(*aActor, hfGeom, aMaterialArray, nbMaterials);
//
//	return ourInstance->CreateShape(hfGeom);
//}
//
//physx::PxShape* Physics::CreateTriangleMesh(const std::vector<CU::Vector3f>& someVerts,
//	const std::vector<unsigned>& someIndices,
//	bool isExclusive)
//{
//	physx::PxTriangleMeshDesc meshDesc;
//	meshDesc.points.count = someVerts.size();
//	meshDesc.points.stride = sizeof(CU::Vector3f);
//	meshDesc.points.data = &someVerts[0];
//
//	meshDesc.triangles.count = someIndices.size() / 3;
//	meshDesc.triangles.stride = sizeof(unsigned int) * 3;
//	meshDesc.triangles.data = &someIndices[0];
//
//	physx::PxDefaultMemoryOutputStream writeBuffer;
//	bool status = ourInstance->myCooking->cookTriangleMesh(meshDesc, writeBuffer);
//	if (!status)
//		return nullptr;
//
//	physx::PxDefaultMemoryInputData readBuffer(writeBuffer.getData(), writeBuffer.getSize());
//	auto* mesh = ourInstance->myPhysics->createTriangleMesh(readBuffer);
//
//	return ourInstance->CreateShape(physx::PxTriangleMeshGeometry(mesh), isExclusive);
//}

bool Physics::InitPVD()
{
	myPvdTransport = physx::PxDefaultPvdSocketTransportCreate("localhost", 5425, 50000);
	assert(myPvdTransport && "Failed to create physics view debugger transport!");

	if (!myPvdTransport)
		return false;

	myPvd = physx::PxCreatePvd(*myPxFoundation);

	return false;
}

bool Physics::InitCooking()
{
	myCooking = PxCreateCooking(
		PX_PHYSICS_VERSION, *myPxFoundation, physx::PxCookingParams(myPxTolerancesScale));
	assert(myCooking);

	return myCooking;
}
