#include "pch.h"
#include "ParticleEmitterComponent.h"

#include "VFXParticleAttraction.h"
#include "Engine/AnimationCurve/Curve.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/Renderer/GraphicsEngine.h"
#include "Engine/Renderer/VertexTypes.h"
#include "Engine/Scene/SceneManager.h"
#include "Engine/ResourceManagement/Resources/MaterialResource.h"
#include "Engine/ResourceManagement/Resources/AnimationCurveResource.h"

Engine::ParticleEmitterComponent::ParticleEmitterComponent(GameObject* aGameObject,
	const VFX::ParticleEmitterDescription& aDescription)
	: Component(aGameObject), myDescription(aDescription), VFX::Emitter(aDescription.base), mySpace(aDescription.base.space)
{
	SubscribeStop([&]
		{
			VFX::Emitter::Stop();
		});
	SubscribeForceStop([&]
		{
			KillInstance();
		});

}

Engine::ParticleEmitterComponent::~ParticleEmitterComponent()
{
}

void Engine::ParticleEmitterComponent::Start()
{
	// TODO: CALCULATE MAX PARTICLES
	// TODO(filip): VIKTOR, i framtiden ska vi INTE ladda från disk i Start()
	// Detta ska redan vara laddat, det är målet.
	// Då får vi lagg spikes i runtime eftersom vi t.ex. spawnar en particle som inte är inladdad
	// för första gången, hemskt.
	// Lösning är att i VFX::Load() där kallar man RequestLoading() på resursen
	// och kallas .AddChild() så att den blir en child till sin parent resurs
	// Annars kommer den inte kunna kolla om den blivit klart inladdad eller inte.
	// Här inne ska du endast gör att assert om den är laddad eller inte

	auto mat = GResourceManager->CreateRef<MaterialResource>(myDescription.particleDescription.materialPath);
	mat->Load();
	myMaterialInstance.Init(mat);

	myParticleStartVariables = myDescription.particleDescription.startVariables;
	myParticleUpdateVariables = myDescription.particleDescription.updateVariables;

	// NOT USED
	//std::string myPath = "Assets/Sprites/UI/bloodSmarrig.dds";
	//if (myParticleStartVariables & VFX::ParticleStartVariables::TEXTURE)
	//{
	//	if (!myDescription.particleDescription.texturePath.empty())
	//	{
	//		myPath = myDescription.particleDescription.texturePath;
	//	}
	//}
	if (myParticleStartVariables & VFX::ParticleStartVariables::SIZE)
	{
		if (myDescription.particleDescription.sizeSetting & VFX::ePropertySetting::eLERP)
		{
			if (!myDescription.particleDescription.sizeCurve.empty())
			{
				mySizeCurve = GResourceManager->CreateRef<AnimationCurveResource>(myDescription.particleDescription.sizeCurve);
				mySizeCurve->Load();
			}
		}
	}

	if (myParticleUpdateVariables & VFX::ParticleUpdateVariables::ACCELERATION)
	{
		if (myDescription.particleDescription.acceleration.settings & VFX::ePropertySetting::eLERP)
		{
			if (!myDescription.particleDescription.acceleration.curve.empty())
			{
				myAcceleratinCurve = GResourceManager->CreateRef<AnimationCurveResource>(myDescription.particleDescription.acceleration.curve);
				myAcceleratinCurve->Load();
			}
		}
	}

	if (myParticleStartVariables & VFX::ParticleStartVariables::MATERIAL)
	{
		if (!myDescription.particleDescription.materialShaderRefPath1.empty())
		{
			myMaterialCurveRef.ref1 = GResourceManager->CreateRef<AnimationCurveResource>(myDescription.particleDescription.materialShaderRefPath1);
			myMaterialCurveRef.ref1->Load();
		}
		if (!myDescription.particleDescription.materialShaderRefPath2.empty())
		{
			myMaterialCurveRef.ref2 = GResourceManager->CreateRef<AnimationCurveResource>(myDescription.particleDescription.materialShaderRefPath2);
			myMaterialCurveRef.ref2->Load();
		}
		if (!myDescription.particleDescription.materialShaderRefPath3.empty())
		{
			myMaterialCurveRef.ref3 = GResourceManager->CreateRef<AnimationCurveResource>(myDescription.particleDescription.materialShaderRefPath3);
			myMaterialCurveRef.ref3->Load();
		}
		if (!myDescription.particleDescription.materialShaderRefPath4.empty())
		{
			myMaterialCurveRef.ref4 = GResourceManager->CreateRef<AnimationCurveResource>(myDescription.particleDescription.materialShaderRefPath4);
			myMaterialCurveRef.ref4->Load();
		}
	}

	if (myParticleUpdateVariables & VFX::ParticleUpdateVariables::GRAVITY)
	{
		myGravity = myDescription.particleDescription.gravity;
	}

	if (myDescription.particleDescription.updateVariables & VFX::ParticleUpdateVariables::EMIT_ON_MOVE)
	{
		myLastEmitPosition = GetTransform().GetPosition();
		myEmitOnMove.nextEmitUnits = myDescription.particleDescription.emitOnMove.units + Random::RandomFloat(-myDescription.particleDescription.emitOnMove.modifier, myDescription.particleDescription.emitOnMove.modifier);
	}

	myParticles.reserve(GetMaxParticles());
	LOG_INFO(LogType::Particles) << "Particle Emitter object created";

	myLastEmitPosition = GetTransform().GetPosition();
}

void Engine::ParticleEmitterComponent::Execute(eEngineOrder aOrder)
{
	if (AutoDestroyCheck())
	{
		KillInstance();
	}
	//myTotalTime += Time::DeltaTime;

	//VFX::Emitter::Tick();

	//if (!VFX::Emitter::IsFinished() && myDescription.particleDescription.updateVariables & VFX::ParticleUpdateVariables::EMIT_ON_MOVE)
	//{
	//	Vec3f dir = GetTransform().GetPosition() - myLastEmitPosition;
	//	const float distance = dir.Length();
	//	dir.Normalize(distance);

	//	float oldUnitsMoved = myEmitOnMove.unitsMoved;
	//	myEmitOnMove.unitsMoved += distance;

	//	Vec3f pos = myLastEmitPosition;
	//	// TODO: Linearly output emits along path from last pos

	//	float dist = myEmitOnMove.nextEmitUnits - oldUnitsMoved;
	//	while (myEmitOnMove.unitsMoved > myEmitOnMove.nextEmitUnits)
	//	{
	//		pos += dir * dist;
	//		myEmitOnMove.unitsMoved -= myEmitOnMove.nextEmitUnits;
	//		myEmitOnMove.nextEmitUnits = myDescription.particleDescription.emitOnMove.units + Random::RandomFloat(-myDescription.particleDescription.emitOnMove.modifier, myDescription.particleDescription.emitOnMove.modifier);

	//		dist = myEmitOnMove.nextEmitUnits;

	//		Emit(pos, GetTransform().GetRotation());
	//	}

	//	myLastEmitPosition = GetTransform().GetPosition();
	//}

	//TickParticles();
	//if (AutoDestroyCheck())
	//{
	//	KillInstance();
	//}
}

void Engine::ParticleEmitterComponent::ExecuteParallelized(eEngineOrder aOrder)
{
	ZoneNamedN(zone2, "ParticleEmitterComponent::ExecuteParallelized", true);
	ZoneTextV(zone2, myDescription.particleDescription.texturePath.c_str(), myDescription.particleDescription.texturePath.size());
	myTotalTime += Time::DeltaTime;

	VFX::Emitter::Tick();

	if (!VFX::Emitter::IsFinished() && myDescription.particleDescription.updateVariables & VFX::ParticleUpdateVariables::EMIT_ON_MOVE)
	{
		Vec3f dir = GetTransform().GetPosition() - myLastEmitPosition;
		const float distance = dir.Length();
		dir.Normalize(distance);

		float oldUnitsMoved = myEmitOnMove.unitsMoved;
		myEmitOnMove.unitsMoved += distance;

		Vec3f pos = myLastEmitPosition;
		// TODO: Linearly output emits along path from last pos

		float dist = myEmitOnMove.nextEmitUnits - oldUnitsMoved;
		while (myEmitOnMove.unitsMoved > myEmitOnMove.nextEmitUnits)
		{
			pos += dir * dist;
			myEmitOnMove.unitsMoved -= myEmitOnMove.nextEmitUnits;
			myEmitOnMove.nextEmitUnits = myDescription.particleDescription.emitOnMove.units + Random::RandomFloat(-myDescription.particleDescription.emitOnMove.modifier, myDescription.particleDescription.emitOnMove.modifier);

			dist = myEmitOnMove.nextEmitUnits;

			Emit(pos, GetTransform().GetRotation());
		}

		myLastEmitPosition = GetTransform().GetPosition();
	}

	TickParticles();
}

void Engine::ParticleEmitterComponent::Render()
{
	// if (myModelInstance.GetModel() && myModelInstance.GetModel()->IsValid())
	// {
	// 
	// }

	if (myMaterialInstance.IsValid())
	{
		myGameObject->GetScene()->GetRendererScene().RenderParticleEmitter(*this);
	}
}

std::vector<ParticleVertex>& Engine::ParticleEmitterComponent::GetVertices()
{
	return myVertices;
}

const VFX::ParticleEmitterDescription& Engine::ParticleEmitterComponent::GetDesc() const
{
	return myDescription;
}

Engine::ParticleMaterialInstance& Engine::ParticleEmitterComponent::GetMaterialInstance()
{
	return myMaterialInstance;
}

void Engine::ParticleEmitterComponent::Emit()
{
	Emit(GetTransform().GetPosition(), GetTransform().GetRotation());
}

void Engine::ParticleEmitterComponent::Emit(const Vec3f& aPosition, const Quatf& aRotation)
{
	if (mySpace == eSpace::WORLDSPACE)
		myDescription.base.geometry.SetRotation(aRotation * myDescription.base.rotation);
	else 
		myDescription.base.geometry.SetRotation(myDescription.base.rotation);

	Particle p;
	p.lifetime = myDescription.particleDescription.lifetime + Random::RandomFloat(-myDescription.particleDescription.lifetimeModifier, myDescription.particleDescription.lifetimeModifier);
	p.color = myDescription.particleDescription.color;
	p.size = myDescription.particleDescription.size;

	if (myParticleStartVariables & VFX::ParticleStartVariables::SIZE)
	{
		if (myDescription.particleDescription.sizeSetting & VFX::ePropertySetting::eMODIFIER)
		{
			auto modifier = myDescription.particleDescription.sizeModifier;

			Vec2f sizeRandom;
			sizeRandom.x = Random::RandomFloat(-modifier.x, modifier.x);
			sizeRandom.y = (modifier.y >= 0) ? Random::RandomFloat(-modifier.y, modifier.y) : sizeRandom.x;

			p.size += sizeRandom;
		}
	}

	if (myParticleStartVariables & VFX::ParticleStartVariables::ROTATION)
		p.rotation = myDescription.particleDescription.startRotation.rotation + Random::RandomFloat(-myDescription.particleDescription.startRotation.modifier, myDescription.particleDescription.startRotation.modifier);

	if (myParticleUpdateVariables & VFX::ParticleUpdateVariables::ROTATE)
		p.rotationSpeed = myDescription.particleDescription.rotate.speed + Random::RandomFloat(-myDescription.particleDescription.rotate.modifier, myDescription.particleDescription.rotate.modifier);

	auto spawn = myDescription.base.geometry.Get();

	if (mySpace == eSpace::WORLDSPACE)
		spawn.position += aPosition;

	p.forward = spawn.forward;
	p.transform.SetPositionLocal(spawn.position);


	if (myParticleStartVariables & VFX::ParticleStartVariables::VELOCITY)
	{
		float velocity = myDescription.particleDescription.startVelocity.force;
		if (myDescription.particleDescription.startVelocity.settings & VFX::ePropertySetting::eMODIFIER)
			velocity += Random::RandomFloat(-myDescription.particleDescription.startVelocity.modifier, myDescription.particleDescription.startVelocity.modifier);
		p.velocity = spawn.forward * velocity;
	}

	myParticles.emplace_back(p);
}

bool Engine::ParticleEmitterComponent::AutoDestroyCheck()
{
	return VFX::Emitter::IsFinished() && (!myDescription.base.keepAlive || (myDescription.base.keepAlive && myParticles.empty()));
}

void Engine::ParticleEmitterComponent::TickParticles()
{
	//myGameObject->GetScene()->GetRendererScene().RenderParticleEmitter(myParticleEmitterInstance);

	// myGameObject->GetScene()->GetRendererScene().RenderParticleEmitter(*this);

	if (mySpace == eSpace::WORLDSPACE)
		myDescription.base.geometry.SetRotation(myGameObject->GetTransform().GetRotation());

	// Debug emitter direction
	{
		/*	const auto& quat = myDescription.base.geometry.GetRotation();
			const auto matrix = quat.ToMatrix();
			const auto forward = matrix.GetForward();*/
			//myGameObject->GetScene()->GetRendererScene().GetDebugDrawer().DrawLine3D(DebugDrawFlags::Always,
			//	myGameObject->GetTransform().GetPosition(),
			//	myGameObject->GetTransform().GetPosition() + forward * 100.f, 0.f,
			//	{ 0, 1, 1, 1 });

		for (int i = static_cast<int>(myParticles.size()) - 1; i >= 0; --i)
		{
			auto& p = myParticles[i];
			p.time += Time::DeltaTime;

			const float percentage = p.time / p.lifetime;
			if (percentage > 1.f)
			{
				myParticles.erase(myParticles.begin() + i);
				continue;
			}

			if (myDescription.particleDescription.colorSetting & VFX::ePropertySetting::eLERP)
			{
				p.color = Math::Lerp(myDescription.particleDescription.color, myDescription.particleDescription.endColor, percentage);
			}

			if (myDescription.particleDescription.startVariables & VFX::ParticleStartVariables::SIZE)
				if (myDescription.particleDescription.sizeSetting & VFX::ePropertySetting::eLERP)
				{
					float magnitude = 1;
					if (mySizeCurve && mySizeCurve->IsValid())
						magnitude = mySizeCurve->Get().Evaluate(percentage);

					p.size = myDescription.particleDescription.size * magnitude;
				}


			if (myParticleUpdateVariables & VFX::ParticleUpdateVariables::GRAVITY)
				p.velocity += myGravity * Time::DeltaTime;

			if (myParticleUpdateVariables & VFX::ParticleUpdateVariables::ACCELERATION)
			{
				Vec3f acceleration = myDescription.particleDescription.acceleration.speed;
				if (myDescription.particleDescription.acceleration.settings & VFX::ePropertySetting::eLERP && myAcceleratinCurve && myAcceleratinCurve->IsValid())
					acceleration *= myAcceleratinCurve->Get().Evaluate(percentage);
				p.velocity += p.forward * acceleration * Time::DeltaTime;
			}

			if (myParticleUpdateVariables & VFX::ParticleUpdateVariables::ROTATE)
				p.rotation += p.rotationSpeed * Time::DeltaTime;

			if (myParticleUpdateVariables & VFX::ParticleUpdateVariables::ATTRACT)
			{
				Vec3f dir;
				if (myDescription.particleDescription.attraction.setting == VFX::eGlobalPoint)
				{
					dir = VFXParticleAttraction::GetAttractionPoint() - p.transform.GetPositionLocal();
				}
				if (myDescription.particleDescription.attraction.setting == VFX::eEmitterLocal)
				{
					dir = (myDescription.base.space == eSpace::WORLDSPACE ? GetTransform().GetPosition() : Vec3f{0,0,0}) - p.transform.GetPositionLocal();
				}

				const float dist = dir.Length();
				if (dist <= myDescription.particleDescription.attraction.collectRange)
				{
					myParticles.erase(myParticles.begin() + i);
					continue;
				}

				if (dist < myDescription.particleDescription.attraction.range)
				{
					//const float distPercentage = dist / myDescription.particleDescription.attraction.range;

					//dir.Normalize(dist);
					//p.velocity += dir * myDescription.particleDescription.attraction.force * distPercentage * Time::DeltaTime;


					//Simon - Lösning som funkar ok, men äter nog prestanda, Viktor får fixa l8r

					const float distPercentage = dist / myDescription.particleDescription.attraction.range;

					dir.Normalize(dist);
					Vec3f change = dir * myDescription.particleDescription.attraction.force * distPercentage * Time::DeltaTime;

					const Vec3f velNormalized = p.velocity.GetNormalized();
					if (velNormalized.Dot(dir) < 0.8f)
					{
						p.velocity -= velNormalized * change.Length();
					}

					p.velocity += change;
				}
			}

			Vec3f movement;
			if (myParticleUpdateVariables & VFX::ParticleUpdateVariables::NOISE)
			{
				const auto& noise = myDescription.particleDescription.noise;

				Vec3f pos = Vec3f(p.transform.GetPositionLocal().x * noise.size + Time::TotalTime * noise.scrollSpeed.x, p.transform.GetPositionLocal().y * noise.size + Time::TotalTime * noise.scrollSpeed.y, p.transform.GetPositionLocal().z * noise.size + Time::TotalTime * noise.scrollSpeed.z) / 100.f;
				
				if (myDescription.particleDescription.noise.setting == VFX::HARD)
				{

					if (noise.force.x < -.001 || noise.force.x > .001)
					{
						float perlin = Math::Lerp(-1, 1, Math::PerlinNoise(-pos.x, pos.y, pos.z));
						movement.x += perlin * noise.force.x * Time::DeltaTime;
					}

					if (noise.force.y < -.001 || noise.force.y > .001)
					{
						float perlin = Math::Lerp(-1, 1, Math::PerlinNoise(pos.x, -pos.y, pos.z));
						movement.y += perlin * noise.force.y * Time::DeltaTime;
					}

					if (noise.force.z < -.001 || noise.force.z > .001)
					{
						float perlin = Math::Lerp(-1, 1, Math::PerlinNoise(pos.x, pos.y, -pos.z));
						movement.z += perlin * noise.force.z * Time::DeltaTime;
					}

					//GDebugDrawer->DrawLine3D(DebugDrawFlags::Particles, p.transform.GetPositionLocal(), p.transform.GetPositionLocal() + movement / Time::DeltaTime, 0, Vec4f(1, 1, 0, 1));
				}

				if (myDescription.particleDescription.noise.setting == VFX::SOFT)
				{
					if (noise.force.x < -.001 || noise.force.x > .001)
					{
						float perlin = Math::Lerp(-1, 1, Math::PerlinNoise(-pos.x, pos.y, pos.z));
						p.velocity.x += perlin * noise.force.x * Time::DeltaTime;
					}

					if (noise.force.y < -.001 || noise.force.y > .001)
					{
						float perlin = Math::Lerp(-1, 1, Math::PerlinNoise(pos.x, -pos.y, pos.z));
						p.velocity.y += perlin * noise.force.y * Time::DeltaTime;
					}

					if (noise.force.z < -.001 || noise.force.z > .001)
					{
						float perlin = Math::Lerp(-1, 1, Math::PerlinNoise(pos.x, pos.y, -pos.z));
						p.velocity.z += perlin * noise.force.z * Time::DeltaTime;
					}
				}
			}

			if (myParticleUpdateVariables & VFX::ParticleUpdateVariables::DRAG) 
			{
				const float c = myDescription.particleDescription.drag.drag;
				const float air = myDescription.particleDescription.drag.airDensity;

				const float velocity = p.velocity.Length();
				const float dragForce = .5f * air * pow(velocity / 100.f, -2) * c;

				Vec3f dirDragForce = p.velocity;
				dirDragForce.Normalize(velocity);

				if (velocity > dragForce) 
				{
					p.velocity -= dirDragForce * dragForce * 100.f * Time::DeltaTime;
				}
				else
				{
					p.velocity = { 0,0,0 };
				}
			}

			movement += p.velocity * Time::DeltaTime;

			p.transform.SetPositionLocal(p.transform.GetPositionLocal() + movement);


			if (myParticleUpdateVariables & VFX::ParticleUpdateVariables::WRAP)
			{
				if (myDescription.particleDescription.wrapPosition)
				{
					auto pos = p.transform.GetPositionLocal();
					switch (myDescription.base.geometry.GetShape())
					{
					case eEmitShape::POINT: break;
					case eEmitShape::SPHERE:
					{
						auto dir = GetTransform().GetPosition() - pos;
						auto radius = myDescription.base.geometry.properties.sphere.radius;
						if (dir.LengthSqr() > radius * radius)
						{
							//const float length = dir.Length();
							//dir.Normalize(length);
							//const float dist = static_cast<int>(length * 0.95f) % static_cast<int>(radius);
							p.transform.SetPositionLocal(GetTransform().GetPosition() + dir * 0.9f);
						}
					}
					break;
					case eEmitShape::BOX:
					{
						auto halfSize = myDescription.base.geometry.properties.box.halfSize;
						auto local = GetTransform().GetPosition() - pos;
						if (local.x < -halfSize.x)
						{
							pos.x -= (halfSize.x * 2.f) * 0.97f;
							p.transform.SetPositionLocal(pos);
						}
						if (local.y < -halfSize.y)
						{
							pos.y -= (halfSize.y * 2.f) * 0.97f;
							p.transform.SetPositionLocal(pos);
						}
						if (local.z < -halfSize.z)
						{
							pos.z -= (halfSize.z * 2.f) * 0.97f;
							p.transform.SetPositionLocal(pos);
						}
						if (local.x > halfSize.x)
						{
							pos.x += (halfSize.x * 2.f) * 0.97f;
							p.transform.SetPositionLocal(pos);
						}
						if (local.y > halfSize.y)
						{
							pos.y += (halfSize.y * 2.f) * 0.97f;
							p.transform.SetPositionLocal(pos);
						}
						if (local.z > halfSize.z)
						{
							pos.z += (halfSize.z * 2.f) * 0.97f;
							p.transform.SetPositionLocal(pos);
						}
					}
					break;
					case eEmitShape::CONE: break;
					default:;
					}

				}
			}

		}
	}

	std::vector<ParticleVertex> vertices(myParticles.size());
	for (int i = 0; i < myParticles.size(); i++)
	{
		ParticleVertex& v = vertices[i];
		auto& p = myParticles[i];

		auto pos = p.transform.GetPositionLocal();

		v.myPosition = Vec4f{ pos.x, pos.y, pos.z, 1.f };

		if (myParticleStartVariables & VFX::ParticleStartVariables::MATERIAL)
		{
			const float percentage = p.time / p.lifetime;

			float c1 = 0, c2 = 0, c3 = 0, c4 = 0;
			if (myMaterialCurveRef.ref1 && myMaterialCurveRef.ref1->IsValid())
				c1 = myMaterialCurveRef.ref1->Get().Evaluate(percentage);
			if (myMaterialCurveRef.ref2 && myMaterialCurveRef.ref2->IsValid())
				c2 = myMaterialCurveRef.ref2->Get().Evaluate(percentage);
			if (myMaterialCurveRef.ref3 && myMaterialCurveRef.ref3->IsValid())
				c3 = myMaterialCurveRef.ref3->Get().Evaluate(percentage);
			if (myMaterialCurveRef.ref4 && myMaterialCurveRef.ref4->IsValid())
				c4 = myMaterialCurveRef.ref4->Get().Evaluate(percentage);

			v.myVelocity = Vec4f{ c1, c2, c3, c4 };
		}
		v.myColor = { p.color.x, p.color.y, p.color.z, p.color.w };
		v.mySize = p.size;
		v.myEmissiveStrength = myDescription.particleDescription.colorIntensity;
		v.myLifetime = p.lifetime;
		v.myRotationRadians = p.rotation * Math::Deg2Rad;

		//GDebugDrawer->DrawCircle3D(DebugDrawFlags::Particles,
		//	GetTransform().GetPosition() + pos, v.mySize.x, 0, v.myColor);
	}

	myVertices = vertices;
	// myParticleEmitterInstance->SetVertices(vertices);

	// NOTE(filip): The sorting is done the render part of the engine, do not 
	// do it on the update thread
	//myParticleEmitterInstance->Update(Time::DeltaTime,
	//	GetEngine()
	//	.GetSceneManager()
	//	.GetActiveScene()
	//	->GetMainCameraGameObject()
	//	->GetTransform()
	//	.GetPosition());

	assert(myParticles.size() <= GetMaxParticles()
		&& "Particle system is not allowed to have more particles than max particles! Set "
		"MaxParticles in emit data");
}

void Engine::ParticleEmitterComponent::KillInstance()
{
	LOG_INFO(LogType::Particles) << "Particle Emitter object destroy";
	DetachVFX();
	myGameObject->Destroy();
}