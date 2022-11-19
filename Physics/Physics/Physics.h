#pragma once

namespace physx
{
	class PxPhysics;
	class PxCooking;
	class PxMaterial;
	class PxScene;
	class PxShape;
	class PxPvD;
	class PxPvDTransport;
	class PxDefaultCpuDispatcher;
	class PxGeometry;
}

class Physics
{
public:
	bool					Init();
	void					SetMaterial(float aStaticFriction, float aDynamicFriction, float aRestitution);
	physx::PxMaterial*		GetMaterial() const;


	physx::PxScene*			CreateScene();
	//physx::PxShape*			CreateShape(const physx::PxGeometry& aGeometry, bool isExclusive = true);
	physx::PxCapsuleController* CreateCharacterController();

	physx::PxPhysics*		GetPhysics();
	physx::PxPvd*			GetPVD();
	static bool				ConnectPVD();

	static Physics&			GetInstance();

	//static physx::PxShape* CreateBox(const CU::Vector3f& aSize, bool isExclusive = true);
	//static physx::PxShape* CreateSphere(float aRadius, bool isExclusive = true);
	//static physx::PxShape* CreateCapsule(float aRadius, float aHalfLength, bool isExclusive = true);
	//static physx::PxShape* CreateConvex(const std::vector<CU::Vector3f>& someVerts, bool isExclusive = true);
	//static physx::PxShape* CreateHeightMap(const int_fast16_t* aHeightMap, int aNumRows, int aNumColums);
	//static physx::PxShape* CreateTriangleMesh(const std::vector<CU::Vector3f>& someVerts, const std::vector<unsigned int>& someIndices, bool isExclusive = true);
private:
	friend class CookingManager;

	bool					InitPVD();
	bool					InitCooking();

	static Physics* ourInstance;

	physx::PxPhysics* myPhysics = nullptr;
	physx::PxFoundation* myPxFoundation = nullptr;


	physx::PxDefaultAllocator		myPxDefaultAllocatorCallback;
	physx::PxDefaultErrorCallback	myPxDefaultErrorCallback;
	physx::PxDefaultCpuDispatcher*	myPxDefaultCpuDispatcher = nullptr;
	physx::PxTolerancesScale		myPxTolerancesScale;

	// Should be removed since we have a material system now.
	physx::PxMaterial* myPxMaterial = nullptr;

	physx::PxPvd* myPvd = nullptr;
	physx::PxPvdTransport* myPvdTransport = nullptr;


	physx::PxCooking* myCooking = nullptr; // Must be furthest down or it crashes


	// TODO:
	// Cooking offline
	// Possibly move info and uuid to all rigidibodies and their information so we don't have to search for it when colliding to get gameobject or anything else
	// 
	// 
	// 
	// 
};

