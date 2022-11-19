#include "pch.h"
#include "Shape.h"

#include "CookingManager.h"
#include "Filters/PhysXFilterShader.h"
#include "Physics.h"
#include "PhysicsMaterialManager.h"
#include "Common/Log.h"

bool Shape::IsValid() const
{
	return myShape != nullptr;
}

void Shape::SetLocalPosition(const CU::Vector3f& aPosition)
{
	auto pose = myShape->getLocalPose();
	pose.p = { aPosition.x, aPosition.y, aPosition.z };
	myShape->setLocalPose(pose);
}

void Shape::SetLocalQuaternion(const CU::Quaternion& aRotation)
{
	auto pose = myShape->getLocalPose();
	pose.q = physx::PxQuat(aRotation.myVector.x, aRotation.myVector.y, aRotation.myVector.z, aRotation.myW);
	myShape->setLocalPose(pose);
}

void Shape::SetupFiltering(unsigned int aFilterGroup, unsigned int aPhysicalFilterMask, unsigned int aDetectionFilterMask)
{
	physx::PxFilterData filterData;
	filterData.word0 = aFilterGroup; // Actors ID 
	filterData.word1 = aPhysicalFilterMask; // Mask for which ID's will activate physical pushback
	filterData.word2 = aDetectionFilterMask; // Mask for which ID's will trigger collision detection

	myShape->setSimulationFilterData(filterData);
	myShape->setQueryFilterData(filterData);
}

void Shape::SetTrigger(bool value)
{
	// Apparently it matters which order so quick fix with ugly ifs
	if (value)
	{
		myShape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, false);
		myShape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, true);
		return;
	}
	myShape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, false);
	myShape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, true);
}

CU::Vector3f Shape::GetLocalPosition() const
{
	auto pose = myShape->getLocalPose();
	return CU::Vector3f(pose.p.x, pose.p.y, pose.p.z);
}

CU::Quaternion Shape::GetLocalQuaternion() const
{
	auto quat = myShape->getLocalPose();
	return CU::Quaternion(quat.q.w, { quat.q.x, quat.q.y, quat.q.z });
}

void Shape::SetName(const std::string& aName)
{
	;
	myShape->setName(aName.c_str());
}

std::string Shape::GetName() const
{
	return myShape->getName();
}

void Shape::SetData(void* aData) const
{
	myShape->userData = aData;
}

void* Shape::GetData() const
{
	return myShape->userData;
}

Shape::eGeometry Shape::GetType() const
{
	return myGeometry;
}

float Shape::GetRadius() const
{
	bool isValid;
	if (myGeometry == eGeometry::eSPHERE) {
		physx::PxSphereGeometry sphere;
		isValid = myShape->getSphereGeometry(sphere);
		assert(isValid && "Illegal check of shape with wrong geometry");
		return sphere.radius;
	}

	if (myGeometry == eGeometry::eCAPSULE) {
		physx::PxCapsuleGeometry capsule;
		isValid = myShape->getCapsuleGeometry(capsule);
		assert(isValid && "Illegal check of shape with wrong geometry");
		return capsule.radius;
	}
	return 1.f;
}

float Shape::GetHeight() const
{
	physx::PxCapsuleGeometry capsule;
	bool isValid = myShape->getCapsuleGeometry(capsule);

	assert(isValid && "Illegal check of shape with wrong geometry");
	if (isValid)
	{
		return capsule.halfHeight;
	}
	return 1.0f;
}

CU::Vector3f Shape::GetBoxHalfSize() const
{
	physx::PxBoxGeometry box;
	bool isValid = myShape->getBoxGeometry(box);
	// assert(isValid && "Illegal check of shape with wrong geometry");
	if (isValid)
	{
		return CU::Vector3f{ box.halfExtents.x, box.halfExtents.y, box.halfExtents.z };
	}
	return { 1, 1, 1 };
}

std::vector<CU::Vector3f> Shape::GetConvexVertices() const
{
	physx::PxConvexMeshGeometry mesh;
	if (!myShape->getConvexMeshGeometry(mesh))
	{
		return {};
	}
	auto size = mesh.convexMesh->getNbVertices();
	auto vertices = mesh.convexMesh->getVertices();

	mesh.convexMesh->getIndexBuffer();

	std::vector<CU::Vector3f> res;
	for (int i = 0; i < size; i++)
	{
		auto v = vertices[i];
		res.emplace_back(CU::Vector3f{ v.x, v.y, v.z });
	}

	return res;
}


Shape Shape::Box(const CU::Vector3f& size, ePhysicsMaterial material, bool isExclusive)
{
	auto shape = Shape();

	auto* pxMaterial = PhysicsMaterialManager::GetMaterial(material);

	physx::PxShape* pxShape = nullptr;

	if (size.x == 0.f || size.y == 0.f || size.z == 0.f)
	{
		float unit = 100.f;
		pxShape = Physics::GetInstance().GetPhysics()->createShape(
			physx::PxBoxGeometry(
				unit * .5f,
				unit * .5f,
				unit * .5f),
			*pxMaterial,
			isExclusive);

		LOG_WARNING(LogType::Physics) << "Shape::Box() size is 0, cannot create shape, creating unit shape";
	}
	else
	{
		pxShape = Physics::GetInstance().GetPhysics()->createShape(physx::PxBoxGeometry(size.x * .5f, size.y * .5f, size.z * .5f), *pxMaterial, isExclusive);
	}

	shape.myShape = pxShape;
	shape.myGeometry = eBOX;
	shape.myIsExclusive = isExclusive;
	shape.myMaterial = material;

	return shape;
}

Shape Shape::Sphere(float radius, ePhysicsMaterial material, bool isExclusive)
{
	auto shape = Shape();

	auto* pxMaterial = PhysicsMaterialManager::GetMaterial(material);

	physx::PxShape* pxShape = nullptr;

	if (radius == 0.f)
	{
		const float unit = 100.f;

		pxShape = Physics::GetInstance().GetPhysics()->createShape(
			physx::PxSphereGeometry(unit),
			*pxMaterial,
			isExclusive);

		LOG_WARNING(LogType::Physics) << "Shape::Sphere() radius is 0, cannot create shape, creating unit shape";
	}
	else
	{
		pxShape = Physics::GetInstance().GetPhysics()->createShape(
			physx::PxSphereGeometry(radius),
			*pxMaterial,
			isExclusive);
	}

	shape.myShape = pxShape;
	shape.myGeometry = eSPHERE;
	shape.myIsExclusive = isExclusive;
	shape.myMaterial = material;

	return shape;
}

Shape Shape::Capsule(float halfHeight, float radius, ePhysicsMaterial material, bool isExclusive)
{
	auto shape = Shape();

	auto* pxMaterial = PhysicsMaterialManager::GetMaterial(material);
	auto* pxShape = Physics::GetInstance().GetPhysics()->createShape(
		physx::PxCapsuleGeometry(radius, halfHeight),
		*pxMaterial,
		isExclusive);

	shape.myShape = pxShape;
	shape.myGeometry = eCAPSULE;
	shape.myIsExclusive = isExclusive;
	shape.myMaterial = material;

	return shape;
}

Shape Shape::Convex(const std::string& aName, ePhysicsMaterial material, bool isExclusive)
{
	Shape shape;

	auto* pxMaterial = PhysicsMaterialManager::GetMaterial(material);
	auto* cookedData = CookingManager::Instance().GetConvex(aName);

	if (!cookedData)
		return shape;

	shape.myShape = Physics::GetInstance().GetPhysics()->createShape(physx::PxConvexMeshGeometry(cookedData), *pxMaterial, isExclusive);
	shape.myGeometry = eGeometry::eCONVEX;
	return shape;
}

Shape Shape::Convex(const std::vector<CU::Vector3f>& someVerts, ePhysicsMaterial material, bool isExclusive)
{
	Shape shape;

	physx::PxConvexMeshDesc convexDesc;
	convexDesc.points.count = someVerts.size();
	convexDesc.points.stride = sizeof(CU::Vector3f);
	convexDesc.points.data = &someVerts[0];
	convexDesc.flags = physx::PxConvexFlag::eCOMPUTE_CONVEX;

	physx::PxDefaultMemoryOutputStream buf;
	if (!CookingManager::Instance().CookConvexMesh(convexDesc, buf))
		return NULL;

	physx::PxDefaultMemoryInputData input(buf.getData(), buf.getSize());
	physx::PxConvexMesh* convexMesh = Physics::GetInstance().GetPhysics()->createConvexMesh(input);

	auto* pxMaterial = PhysicsMaterialManager::GetMaterial(material);

	shape.myShape = Physics::GetInstance().GetPhysics()->createShape(physx::PxConvexMeshGeometry(convexMesh), *pxMaterial, isExclusive);
	shape.myGeometry = eGeometry::eCONVEX;
	return shape;
}

Shape Shape::TriangleMesh(const std::string& aName, ePhysicsMaterial material, bool isExclusive)
{
	Shape shape;

	auto* pxMaterial = PhysicsMaterialManager::GetMaterial(material);
	auto* cookedData = CookingManager::Instance().GetTriangleMesh(aName);

	if (!cookedData)
		return shape;

	shape.myShape = Physics::GetInstance().GetPhysics()->createShape(physx::PxTriangleMeshGeometry(cookedData), *pxMaterial, isExclusive);
	shape.myGeometry = eGeometry::eTRIANGLE_MESH;
	return shape;
}

Shape Shape::TriangleMesh(const std::vector<CU::Vector3f>& someVerts,
	const std::vector<unsigned>& someIndices, ePhysicsMaterial material, bool isExclusive)
{
	Shape shape;
	physx::PxTriangleMeshDesc meshDesc;
	meshDesc.points.count = someVerts.size();
	meshDesc.points.stride = sizeof(CU::Vector3f);
	meshDesc.points.data = &someVerts[0];

	meshDesc.triangles.count = someIndices.size() / 3;
	meshDesc.triangles.stride = sizeof(unsigned int) * 3;
	meshDesc.triangles.data = &someIndices[0];

	physx::PxDefaultMemoryOutputStream writeBuffer;
	bool status = CookingManager::Instance().CookTriangleMesh(meshDesc, writeBuffer);
	if (!status)
		return nullptr;

	physx::PxDefaultMemoryInputData readBuffer(writeBuffer.getData(), writeBuffer.getSize());
	auto* mesh = Physics::GetInstance().GetPhysics()->createTriangleMesh(readBuffer);

	auto* pxMaterial = PhysicsMaterialManager::GetMaterial(material);

	physx::PxMeshPreprocessingFlag::eWELD_VERTICES,

		shape.myShape = Physics::GetInstance().GetPhysics()->createShape(physx::PxTriangleMeshGeometry(mesh), *pxMaterial, isExclusive);
	shape.myGeometry = eGeometry::eTRIANGLE_MESH;
	return shape;
}

Shape Shape::TriangleMeshPath(const std::string& aPath, ePhysicsMaterial material /*= ePhysicsMaterial::Default*/, bool isExclusive /*= true*/)
{
	Shape shape;

	auto* pxMaterial = PhysicsMaterialManager::GetMaterial(material);
	auto* cookedData = CookingManager::Instance().GetTriangleMeshPath(aPath);

	if (!cookedData)
		return shape;

	shape.myShape = Physics::GetInstance().GetPhysics()->createShape(physx::PxTriangleMeshGeometry(cookedData), *pxMaterial, isExclusive);
	shape.myGeometry = eGeometry::eTRIANGLE_MESH;
	return shape;

}

Shape Shape::Copy(Shape& aShape)
{
	Shape shape;
	shape.myGeometry = aShape.myGeometry;

	if (!aShape.myShape)
	{
		LOG_ERROR(LogType::Physics) << "Invalid PhysX shape!";
		return Shape::Box({ 100, 100, 100 });
	}

	auto* pxMaterial = PhysicsMaterialManager::GetMaterial(aShape.myMaterial);
	switch (aShape.myGeometry)
	{
	case eBOX:
	{
		shape.myShape = Physics::GetInstance().GetPhysics()->createShape(aShape.myShape->getGeometry().box(), *pxMaterial, true);
	}
	break;
	case eSPHERE:
		shape.myShape = Physics::GetInstance().GetPhysics()->createShape(aShape.myShape->getGeometry().sphere(), *pxMaterial, true);
		break;
	case eCAPSULE:
		shape.myShape = Physics::GetInstance().GetPhysics()->createShape(aShape.myShape->getGeometry().capsule(), *pxMaterial, true);
		break;
	case eCONVEX:
		shape.myShape = Physics::GetInstance().GetPhysics()->createShape(aShape.myShape->getGeometry().convexMesh(), *pxMaterial, true);
		break;
	case eTRIANGLE_MESH:
		shape.myShape = Physics::GetInstance().GetPhysics()->createShape(aShape.myShape->getGeometry().triangleMesh(), *pxMaterial, true);
		break;
	default:
		break;
	}

	shape.SetLocalQuaternion(aShape.GetLocalQuaternion());
	shape.SetLocalPosition(aShape.GetLocalPosition());

	return shape;
}

// FOR CREATING TRIANGLE MESH - Terrain generator.cpp

//
//BinaryReader reader;
//
//if (!reader.InitWithFile(aPath))
//{
//	return nullptr;
//}
//
//reader.UncompressGzip();
//
//const auto mdl = MyGame::Sample::GetModel(reader.GetStart());
//
//
//std::vector<unsigned int> indices(mdl->indices_count());
//const unsigned int* indicesRaw = mdl->indices()->data();
//for (int i = 0; i < mdl->indices_count(); i++)
//{
//	indices[i] = indicesRaw[i];
//}
//
//std::vector<Vec3f> vertices(mdl->vertex_count());
//const DefaultVertex* verticesArr = reinterpret_cast<const DefaultVertex*>(mdl->vertices()->data());
//for (int i = 0; i < mdl->vertex_count(); i++)
//{
//	vertices[i] = Vec3f(verticesArr[i].myPosition.x, verticesArr[i].myPosition.y, verticesArr[i].myPosition.z);
//}
//	//physx::PxShape* shape = Physics::CreateTriangleMesh(vertices, indices);
//	