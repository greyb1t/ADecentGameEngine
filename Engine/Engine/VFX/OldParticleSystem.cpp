#include "pch.h"
#include "OldParticleSystem.h"

#include "Engine/Scene/Scene.h"
#include "Engine/Scene/SceneManager.h"
#include "../Engine.h"
#include "Engine/DebugManager/DebugDrawer.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/Renderer/GraphicsEngine.h"
#include "Engine/Renderer/Scene/RendererScene.h"
#include "Engine/Renderer/VertexTypes.h"

Transform& Engine::OldParticleSystem::GetTransform()
{
	return myTransform;
}

Engine::OldParticleSystem::~OldParticleSystem()
{
	// myScene->GetRendererScene().RemoveParticleEmitter(myParticleEmitterInstance);
}

void Engine::OldParticleSystem::Init(
	const std::string& aPath, const ParticleEmitData& aData, float aLifeTime)
{
	myPath = aPath;
	myEmitData = aData;
	myTotalLifeTime = aLifeTime;

	myMaxParticles = myEmitData.MaxParticles;
	if (myEmitData.MaxParticles < 0)
	{
		if (myEmitData.EmitAllAtSpawn)
		{
			myMaxParticles = myEmitData.ParticlesPerSecond + 10;
		}
		else
		{
			myMaxParticles = myEmitData.ParticlesPerSecond * myEmitData.LifeTime + 10;
		}
	}

	myParticles.reserve(myMaxParticles);
}

void Engine::OldParticleSystem::Start()
{
	//myParticleEmitterInstance
	//	= GetEngine().GetGraphicsEngine().GetParticleEmitterFactory().GetParticleEmitter(
	//		std::wstring(myPath.begin(), myPath.end()), myMaxParticles);
	// myScene->GetRendererScene().AddParticleEmitter(myParticleEmitterInstance);

	myTimeToSpawn = 1.f / myEmitData.ParticlesPerSecond;

	myEmitData.EmitterGeometry.SetRotation(myTransform.GetRotation());

	if (myEmitData.EmitAllAtSpawn)
	{
		for (int i = 0; i < myEmitData.ParticlesPerSecond; i++)
		{
			Emit();
		}
	}
}

void Engine::OldParticleSystem::Update(float aDeltaTime)
{
	//Engine::Engine::GetInstance().GetDebugDrawer().DrawCube3D(myTransform.GetPosition(),
	//GetTransform().GetRotation().EulerAngles(),
	//{ 50, 100, 50 }, .0f, { 1, 0, 0, 1 });

	// myScene->GetRendererScene().RenderParticleEmitter(*myParticleEmitterInstance);

	myEmitData.EmitterGeometry.SetRotation(myTransform.GetRotation());

	// Debug emitter direction
	{
		auto& quat = myEmitData.EmitterGeometry.GetRotation();
		auto matrix = quat.ToMatrix();
		auto forward = matrix.GetForward();
		myScene->GetRendererScene().GetDebugDrawer().DrawLine3D(DebugDrawFlags::Particles,
			myTransform.GetPosition(),
			myTransform.GetPosition() + forward * 200.f,
			0,
			{0, 1, 1, 1});
	}

	/*{
		CU::Quaternion rotation = CU::Quaternion(CU::Vector3f(
			myEmitData.EmitterGeometry.myRotation.EulerAngles().x 
			+ myEmitData.EmitterGeometry.myConeAngle * .5f * U::Deg2Rad,

			myEmitData.EmitterGeometry.myRotation.EulerAngles().y,

			myEmitData.EmitterGeometry.myRotation.EulerAngles().z 
			+ myEmitData.EmitterGeometry.myConeAngle * .5f * U::Deg2Rad)
		);

		auto matrix = rotation.ToMatrix();
		auto forward = matrix.GetForward();
		GetEngine().GetDebugDrawer().DrawLine3D(myTransform.GetPosition(), myTransform.GetPosition() + forward * 200.f, 0, { 1, 0, 1, 1 });
	}

	{
		CU::Quaternion rotation = CU::Quaternion(CU::Vector3f(
			myEmitData.EmitterGeometry.myRotation.EulerAngles().x
			+ myEmitData.EmitterGeometry.myConeAngle * -.5f * U::Deg2Rad,

			myEmitData.EmitterGeometry.myRotation.EulerAngles().y,

			myEmitData.EmitterGeometry.myRotation.EulerAngles().z
			+ myEmitData.EmitterGeometry.myConeAngle * -.5f * U::Deg2Rad)
		);

		auto matrix = rotation.ToMatrix();
		auto forward = matrix.GetForward();
		GetEngine().GetDebugDrawer().DrawLine3D(myTransform.GetPosition(), myTransform.GetPosition() + forward * 200.f, 0, { 1, 0, 1, 1 });
	}*/
	//

	myLifeTime += aDeltaTime;
	if (myLifeTime > myTotalLifeTime
		&& (!myEmitData.FinishSimulationBeforeDestroy || myParticles.empty()))
	{
		Destroy();
	}

	myTimer += aDeltaTime;
	if (myTimer > myTimeToSpawn && !(myLifeTime > myTotalLifeTime) && !myEmitData.EmitAllAtSpawn)
	{
		while (myTimer > myTimeToSpawn)
		{
			myTimer -= myTimeToSpawn;
			Emit();
		}
	}

	for (int i = static_cast<int>(myParticles.size()) - 1; i >= 0; --i)
	{
		auto& p = myParticles[i];

		p.myLifeTime += aDeltaTime;

		float percentage = p.myLifeTime / p.myTotalLifeTime;
		if (percentage > 1.f)
		{
			myParticles.erase(myParticles.begin() + i);
			continue;
		}

		p.myVelocity += myEmitData.Gravity * aDeltaTime;
		if (myEmitData.VelocityAccelerationCurve.Keys() > 0)
			p.myVelocity += myEmitData.VelocityAccelerationCurve.Value(percentage) * aDeltaTime;

		if (myEmitData.DampeningCurve.Keys() > 0)
		{
			const float dampeningAmnt = myEmitData.DampeningCurve.Value(percentage) * aDeltaTime;
			const float length = p.myVelocity.Length();
			float lengthAfterDampening = length - dampeningAmnt;

			if (lengthAfterDampening < 0.f)
				lengthAfterDampening = 0;

			p.myVelocity *= lengthAfterDampening / length;
		}

		if (myEmitData.VelocityCurve.Keys() > 0)
		{
			const Vec3f velocity = myEmitData.VelocityCurve.Value(percentage);
			p.myVelocity += velocity - p.myLastVelocityCurveValue;
			p.myLastVelocityCurveValue = velocity;
		}

		auto movement
			= p.myForward * p.myVelocity.z + p.myRight * p.myVelocity.x + p.myUp * p.myVelocity.y;

		p.myPosition += movement * aDeltaTime;

		///NOT IN PARTICLEEMITTER RIGHT NOW

		//// Rotation
		//
		//if (myEmitData.RotationAccelerationCurve.Keys() > 0)
		//	p.myVertex. += myEmitData.RotationAccelerationCurve.Value(percentage) * aDeltaTime;
		//
		//float rotation = p.myVertex.;
		//
		//if (myEmitData.RotationCurve.Keys() > 0)
		//	rotation += myEmitData.RotationCurve.Value(percentage);

		Vec2f size = myEmitData.Size;
		if (myEmitData.SizeCurve.Keys() > 0)
		{
			size = size * myEmitData.SizeCurve.Value(percentage);
		}
		p.mySize = size;

		if (myEmitData.ColorCurve.Keys() > 0)
			p.myColor = myEmitData.ColorCurve.Value(percentage);
	}

	std::vector<ParticleVertex> vertices(myParticles.size());

	for (int i = 0; i < myParticles.size(); i++)
	{
		ParticleVertex& v = vertices[i];
		auto& p = myParticles[i];
		v.myPosition = {p.myPosition.x, p.myPosition.y, p.myPosition.z, 1.f};
		v.myColor = {p.myColor.x, p.myColor.y, p.myColor.z, p.myColor.w};
		v.mySize = {p.mySize.x, p.mySize.y};
		v.myVelocity = {p.myVelocity.x, p.myVelocity.y, p.myVelocity.z, 1.f};
		v.myEmissiveStrength = 1.f;
		v.myLifetime = p.myLifeTime;
		v.myDistanceToCamera = 0.f;
	}

	// myParticleEmitterInstance->SetVertices(vertices);

	// NOTE(filip): The sorting is done the render part of the engine, do not 
	// do it on the update thread
	//myParticleEmitterInstance->Update(aDeltaTime,
	//	Engine::GetInstance()
	//		.GetSceneManager()
	//		.GetActiveScene()
	//		->GetMainCameraGameObject()
	//		->GetTransform()
	//		.GetPosition());

	assert(myParticles.size() <= myMaxParticles
		&& "Particle system is not allowed to have more particles than max particles! Set "
		   "MaxParticles in emit data");
}

void Engine::OldParticleSystem::Emit()
{
	Particle p;
	p.myTotalLifeTime = myEmitData.LifeTime;
	p.myLifeTime = 0;

	EmitterGeometry::EmitSpawn spawn = myEmitData.EmitterGeometry.Get();

	p.myPosition = myTransform.GetPosition() + spawn.position;
	p.myForward = spawn.rotation.ToMatrix().GetForward();
	p.myRight = spawn.rotation.ToMatrix().GetRight();
	p.myUp = spawn.rotation.ToMatrix().GetUp();

	myParticles.emplace_back(p);
}