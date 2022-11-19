#pragma once

#include "Component.h"
#include "Engine/VFX/Description/ParticleEmitterDescription.h"
#include "Engine/VFX/Emitter/Emitter.h"
#include "Engine/Renderer/VertexTypes.h"
#include "Engine/VFX/VFXChild.h"
#include "Engine/Renderer/Material/ParticleMaterialInstance.h"
#include "Engine/EngineOrder.h"

namespace Engine
{
	class ParticleEmitterComponent :
		public Component, public VFX::Emitter, public VFX::VFXChild
	{
	public:
		COMPONENT_MASK(ParticleEmitterComponent, "ParticleEmitterComponent", eEngineOrder::EARLY_RENDER);
		ParticleEmitterComponent() = default;
		ParticleEmitterComponent(GameObject* aGameObject, const VFX::ParticleEmitterDescription& aDescription);
		virtual ~ParticleEmitterComponent();

		void Start() override;
		void Execute(eEngineOrder aOrder) override;
		void ExecuteParallelized(eEngineOrder aOrder) override;

		void Render() override;

		std::vector<ParticleVertex>& GetVertices();
		const VFX::ParticleEmitterDescription& GetDesc() const;

		ParticleMaterialInstance& GetMaterialInstance();

		int GetMaxParticles() const { return myDescription.particleDescription.maxParticles; }

	protected:
		void Emit() override;
		void Emit(const Vec3f& aPosition, const Quatf& aRotation);
		bool AutoDestroyCheck() override;
		void TickParticles();
		void KillInstance();
	private:
		struct Particle
		{
			LiteTransform transform;
			Vec4f color;
			Vec2f size;
			Vec3f velocity;
			Vec3f forward;
			float rotation = 0;

			float rotationSpeed = 0;

			float lifetime = 1.f;
			float time = 0;
		};

		VFX::ParticleEmitterDescription myDescription;

		std::vector<ParticleVertex> myVertices;

		ParticleMaterialInstance myMaterialInstance;
		struct
		{
			AnimationCurveRef ref1;
			AnimationCurveRef ref2;
			AnimationCurveRef ref3;
			AnimationCurveRef ref4;
		} myMaterialCurveRef;

		AnimationCurveRef mySizeCurve;
		AnimationCurveRef myAcceleratinCurve;

		std::vector<Particle> myParticles;

		unsigned myParticleStartVariables = 0;
		unsigned myParticleUpdateVariables = 0;
		Vec3f myGravity;
		eSpace mySpace = eSpace::LOCALSPACE;

		Vec3f myLastEmitPosition;

		struct
		{
			float unitsMoved = 0;
			float nextEmitUnits = 0;
		} myEmitOnMove;

		float myTotalTime = 0;

	};
}
