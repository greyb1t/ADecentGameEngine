#pragma once

#include "Engine/GameObject/Components/Component.h"

namespace LevelBossStates
{
	class BossMortar : public Component
	{
		COMPONENT(BossMortar, "Boss Mortar");
	public:
		BossMortar() = default;
		~BossMortar();

		void Awake() override;
		void Reflect(Engine::Reflector& aReflector) override;

		void Execute(Engine::eEngineOrder aOrder) override;

		void SetPositions(const Vec3f& anStartPosition, const float aHeight, const float aTime);

		void SetDamage(const float anAmount);
		void SetClusterCount(const int aCount);
		void SetOffsetSpread(const float aSpread);
		void SetOffsetLanding(const float aValue);
		void SetClusterRadius(const float aRadius);

	private:
		Engine::AudioComponent* myAudioComponent = nullptr;
		VFXRef myExplosionVFXRef;
		GameObjectPrefabRef myClusterPrefab;
		Vec3f myStartPosition = {};
		Vec3f myEndPosition = {};
		Vec3f myLastFramePosition = {};
		float myHeight = 0.0f;

		float myDuration = 0.0f;
		float myProgress = 0.0f;

		int myClusterCount = 20;
		float myOffsetSpread = 1000.f;
		float myOffsetLanding = 0.1f;
		float myClusterRadius = 100.f;
		float myDamage = 10.0f;
	};
}
namespace LBS = LevelBossStates;
