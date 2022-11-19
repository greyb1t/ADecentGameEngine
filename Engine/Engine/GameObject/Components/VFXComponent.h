#pragma once
#include "Component.h"
#include "Engine/VFX/VFXSystem.h"

namespace Engine
{
	class VFXComponent :
		public Component
	{
	public:
		COMPONENT(VFXComponent, "__VFXComponent");
		VFXComponent() = default;
		VFXComponent(GameObject* aGameObject, const VFX::VFXDescription& aDescription);
		void Start() override;
		void Execute(eEngineOrder aOrder) override;

		void Play();
		void Pause();
		// Stops more from spawning
		void Stop();
		void ForceStop();
		void SetDuration(float aDuration);

		// Automatically destroys gameobject it's attached to once it is done.
		void AutoDestroy(bool = true);

		VFX::VFXSystem& GetVFX();

		const bool IsPlaying();

		//VFXComponent* Clone() const override { return new VFXComponent(*this); }

	private:
		void CreateEmitter(const VFX::ParticleEmitterDescription& aDescription);
		void CreateEmitter(const VFX::PrefabEmitterDescription& aDescription);
	private:
		VFX::VFXSystem myVFXSystem;
		float myCustomDuration = -1.f;
		bool myIsAutoDestroy = false;
	};
}
