#pragma once


enum class eEmitShape
{
	POINT,
	SPHERE,
	BOX,
	CONE,
};

enum class eEmitFrom
{
	EDGE,
	VOLUME,
	ORIGIN,
};

enum class eEmitDirection
{
	DEFAULT, // Default refers to forward in object space
	OUTWARDS, // LocalForward refers from origin to particle spawn point.
	RANDOM,
};

namespace Engine {
	class OldParticleSystem;
}

class EmitterGeometry
{
public:
	struct EmitSpawn
	{
		Vec3f position;
		Quatf rotation;
		Vec3f forward;
	};

	struct SphereProperties
	{
		float radius = 100.f;
		float minRadius = 0.f;
	};
	struct BoxProperties
	{
		Vec3f halfSize = { 100, 100, 100 };
	};
	struct ConeProperties
	{
		float angle = 45.f;
		float radius = 100.f;
	};
public:
	union Properties
	{
		SphereProperties sphere;
		BoxProperties box;
		ConeProperties cone;

		Properties() { memset(this, 0, sizeof(*this)); }
	} properties;

	EmitSpawn Get();

	void			SetRotation(const Quatf& aRotation);
	void			SetShape(eEmitShape);
	void			SetFrom(eEmitFrom);
	void			SetDirection(eEmitDirection);


	const Quatf&	GetRotation();
	eEmitShape		GetShape() const;
	eEmitFrom		GetEmitFrom() const;
	eEmitDirection	GetEmitDirection() const;

	static EmitterGeometry Box(const Vec3f& aHalfSize, eEmitFrom aEmitFrom = eEmitFrom::VOLUME, eEmitDirection aEmitDirection = eEmitDirection::OUTWARDS);
	static EmitterGeometry Cone(float aRadius, float anAngle, eEmitFrom aEmitFrom = eEmitFrom::ORIGIN, eEmitDirection aEmitDirection = eEmitDirection::OUTWARDS);
	static EmitterGeometry Sphere(float aRadius, float aMinRadius = 0.f, eEmitFrom aEmitFrom = eEmitFrom::VOLUME, eEmitDirection aEmitDirection = eEmitDirection::OUTWARDS);
private:
	friend class Engine::OldParticleSystem;
	friend class VFXEditor;
	friend class VFXSystem;

	EmitSpawn CalculateSphere();
	EmitSpawn CalculateCone();
	EmitSpawn CalculateBox();

private:
	eEmitShape myEmitShape = eEmitShape::POINT;
	eEmitFrom myEmitFrom = eEmitFrom::VOLUME;
	eEmitDirection myEmitDirection = eEmitDirection::OUTWARDS;
	Quatf myRotation;
};