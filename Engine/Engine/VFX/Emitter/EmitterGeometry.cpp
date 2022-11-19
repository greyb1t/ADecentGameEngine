#include "pch.h"
#include "EmitterGeometry.h"
#include "Common/Random.h"
#include "Engine/DebugManager/DebugDrawer.h"
#include "Engine/DebugManager/DebugDrawerSettings.h"

EmitterGeometry::EmitSpawn EmitterGeometry::Get()
{
	switch (myEmitShape)
	{
	case eEmitShape::POINT:
		return EmitSpawn{ {0,0,0 },Quatf() };
	case eEmitShape::SPHERE:
		return CalculateSphere();
	case eEmitShape::BOX:
		return CalculateBox();
	case eEmitShape::CONE: 
		return CalculateCone();
	default: ;
	}

	return EmitSpawn{ {0,0,0}, Quatf() };
}

void EmitterGeometry::SetRotation(const Quatf& aRotation)
{
	myRotation = aRotation;
}

void EmitterGeometry::SetShape(eEmitShape aShape)
{
	myEmitShape = aShape;
}

void EmitterGeometry::SetFrom(eEmitFrom aFrom)
{
	myEmitFrom = aFrom;
}

void EmitterGeometry::SetDirection(eEmitDirection aDirection)
{
	myEmitDirection = aDirection;
}

const Quatf& EmitterGeometry::GetRotation()
{
	return myRotation;
}

eEmitShape EmitterGeometry::GetShape() const
{
	return myEmitShape;
}

eEmitFrom EmitterGeometry::GetEmitFrom() const
{
	return myEmitFrom;
}

eEmitDirection EmitterGeometry::GetEmitDirection() const
{
	return myEmitDirection;
}

EmitterGeometry EmitterGeometry::Box(const Vec3f& aHalfSize, eEmitFrom aEmitFrom, eEmitDirection aEmitDirection)
{
	EmitterGeometry out;
	out.myEmitShape = eEmitShape::BOX;
	out.myEmitFrom = aEmitFrom;
	out.myEmitDirection = aEmitDirection;
	out.properties.box.halfSize = aHalfSize;

	return out;
}

EmitterGeometry EmitterGeometry::Cone(float aRadius, float anAngle, eEmitFrom aEmitFrom, eEmitDirection aEmitDirection)
{
	EmitterGeometry out;
	out.myEmitShape = eEmitShape::CONE;
	out.myEmitFrom = aEmitFrom;
	out.myEmitDirection = aEmitDirection;
	out.properties.cone.radius = aRadius;
	out.properties.cone.angle = anAngle;

	return out;
}

EmitterGeometry EmitterGeometry::Sphere(float aRadius, float aMinRadius, eEmitFrom aEmitFrom, eEmitDirection aEmitDirection)
{
	EmitterGeometry out;
	out.myEmitShape = eEmitShape::SPHERE;
	out.myEmitFrom = aEmitFrom;
	out.myEmitDirection = aEmitDirection;
	out.properties.sphere.radius = aRadius;
	out.properties.sphere.minRadius = aMinRadius;
	return out;
}

EmitterGeometry::EmitSpawn EmitterGeometry::CalculateSphere()
{
	EmitSpawn out;
	float spawnDistance = 0;

	switch (myEmitFrom) {
	case eEmitFrom::VOLUME:
		spawnDistance = Random::RandomFloat(properties.sphere.minRadius, properties.sphere.radius);
		break;
	case eEmitFrom::EDGE:
		spawnDistance = properties.sphere.radius;
		break;
	case eEmitFrom::ORIGIN:
		break;
	default:
		break;
	}

	Quatf rotation = Quatf(Vec3f(Random::RandomFloat(0, 360), Random::RandomFloat(0, 360), Random::RandomFloat(0, 360)));

	Mat4f matrix = rotation.ToMatrix();
	auto forward = matrix.GetForward();
	out.position = forward * spawnDistance;

	switch (myEmitDirection) {
	case eEmitDirection::DEFAULT:
		out.forward = myRotation.ToMatrix().GetForward();
		break;
	case eEmitDirection::OUTWARDS:
	{
		out.forward = forward;
	}
		break;
	case eEmitDirection::RANDOM:
	{
		Quatf randomDir = Quatf(Vec3f(Random::RandomFloat(0, 360), Random::RandomFloat(0, 360), Random::RandomFloat(0, 360)));
		out.forward = randomDir.ToMatrix().GetForward();
	}
		break;
	default:
		break;
	}

	return out;
}

EmitterGeometry::EmitSpawn EmitterGeometry::CalculateCone()
{
	EmitSpawn out;
	float spawnDistance = 0;

	switch (myEmitFrom) {
	case eEmitFrom::VOLUME:
		spawnDistance = Random::RandomFloat(0, properties.cone.radius);
		break;
	case eEmitFrom::EDGE:
		spawnDistance = properties.cone.radius;
		break;
	case eEmitFrom::ORIGIN:
		break;
	default:
		break;
	}

	Vec3f rotDir = Vec3f(
		Random::RandomFloat(-1, 1), 
		Random::RandomFloat(-1, 1), 
		Random::RandomFloat(-1, 1));
	if (rotDir.LengthSqr() > 1)
		rotDir.Normalize();
		
	Quatf rotation = Quatf(Vec3f(
		rotDir.x * properties.cone.angle * .5f * Math::Deg2Rad,
		rotDir.y * properties.cone.angle * .5f * Math::Deg2Rad,
		rotDir.z * properties.cone.angle * .5f * Math::Deg2Rad)
	);

	Mat4f matrix = rotation.ToMatrix();
	//matrix *= CU::Matrix4f::CreateRotationAroundX(myRotation.EulerAngles().x) * CU::Matrix4f::CreateRotationAroundY(myRotation.EulerAngles().y * Math::Rad2Deg) * CU::Matrix4f::CreateRotationAroundZ(myRotation.EulerAngles().z);
	
	rotation = Quatf(matrix);

	auto forward = matrix.GetForward() * myRotation;
	out.position = forward * spawnDistance;

	out.rotation = Quatf(Mat4f::LookAt({0, 0, 1 }, forward, {0, 1, 0}));

	//GDebugDrawer->DrawLine3D(DebugDrawFlags::Particles,
	//	Vec3f(),
	//	forward * 500.f,
	//	0,
	//	Vec4f(1, 0, 0, 1));

	//GDebugDrawer->DrawLine3D(DebugDrawFlags::Particles,
	//	Vec3f(),
	//	myRotation.ToMatrix().GetForward() * 500.f,
	//	.5f,
	//	Vec4f(0, 1, 0, 1));



	switch (myEmitDirection) {
	case eEmitDirection::DEFAULT:
		out.forward = { 0, 0, 1 };
		break;
	case eEmitDirection::OUTWARDS:
	{
		out.forward = forward;
	}
	break;
	case eEmitDirection::RANDOM:
	{
		Quatf randomDir = Quatf(Vec3f(Random::RandomFloat(0, 360), Random::RandomFloat(0, 360), Random::RandomFloat(0, 360)));
		out.forward = randomDir.ToMatrix().GetForward();
	}
	break;
	default:
		break;
	}

	return out;
}

EmitterGeometry::EmitSpawn EmitterGeometry::CalculateBox()
{
	// TODO add eEmitFrom option

	EmitSpawn out;

	out.position =
		Vec3f(
			Random::RandomFloat(-properties.box.halfSize.x, properties.box.halfSize.x),
			Random::RandomFloat(-properties.box.halfSize.y, properties.box.halfSize.y),
			Random::RandomFloat(-properties.box.halfSize.z, properties.box.halfSize.z)
		);

	switch (myEmitDirection) {
	case eEmitDirection::DEFAULT:
		out.forward = { 0, 0, 1 };
		break;
	case eEmitDirection::OUTWARDS:
		out.forward = out.position.GetNormalized();
		break;
	case eEmitDirection::RANDOM:
	{
		Quatf randomDir = Quatf(Vec3f(Random::RandomFloat(0, 360), Random::RandomFloat(0, 360), Random::RandomFloat(0, 360)));
		out.forward = randomDir.ToMatrix().GetForward();
	}
		break;
	}

	return out;
}
