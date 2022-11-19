#pragma once
#include "../GameObject/GameObject.h"
#include "Common/AnimationCurve.h"
#include "Common/Enums.hpp"
#include "Emitter/EmitterGeometry.h"

struct ParticleEmitData
{
	EmitterGeometry					EmitterGeometry;
	bool							FinishSimulationBeforeDestroy = true;
	bool							EmitAllAtSpawn = false; // Temporary variable to activate only one emit at start of particlesystem with particles per second amount spawned

	float							RotationModifier = 0.f;

	int								MaxParticles = -1;

	float							ParticlesPerSecond = 1.f;
	float							ParticlesPerUnit = 0.f;
	float							LifeTime = 1.f;

	Vec3f					Gravity = { 0, 0, 0 };
	Vec2f					Size = { 100, 100 };
	Vec3f					Pivot = { .5f, .5f, .5f };


	AnimationCurve<Vec3f>	VelocityCurve;
	AnimationCurve<Vec3f>	VelocityAccelerationCurve;
	AnimationCurve<Vec3f>	RotationCurve;
	AnimationCurve<Vec3f>	RotationAccelerationCurve;
	AnimationCurve<C::Vector4f>	ColorCurve;
	AnimationCurve<Vec2f>	SizeCurve;
	AnimationCurve<float>			DampeningCurve;
};

namespace Engine
{
	class ParticleEmitterInstance;
}

namespace Engine
{
	class OldParticleSystem : public GameObject
	{
	public:
		~OldParticleSystem();

		void Init(const std::string& aPath, const ParticleEmitData& aData, float aLifeTime);
		void Start() override;

		void Update(float aDeltaTime) override;

		Transform& GetTransform();
	private:
		void Emit();

	private:
		struct Particle
		{
			float myTotalLifeTime = 1.f;
			float myLifeTime = 0;

			Vec3f myVelocity;
			Vec3f myLastVelocityCurveValue;

			Vec3f myPosition;
			Vec3f myForward	= { 0, 0, 1.f };
			Vec3f myUp		= { 0, 1.f, 0 };
			Vec3f myRight	= { 1.f, 0, 0 };

			C::Vector4f myColor;
			Vec2f mySize;
		};


		ParticleEmitData myEmitData;
		std::string myPath;

		bool myHasEmittedOnce = false; // Only temporary fix for making spawning of all particles at once possible

		float myTimer = 0;
		float myTimeToSpawn = 1.f;

		int myMaxParticles = 64;

		float myLifeTime = 0.f;
		float myTotalLifeTime = 1.f;
		
		Shared<ParticleEmitterInstance> myParticleEmitterInstance;

		std::vector<Particle> myParticles;
	};
}
