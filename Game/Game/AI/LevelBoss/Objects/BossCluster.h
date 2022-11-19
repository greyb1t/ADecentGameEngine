#pragma once

#include "Engine/GameObject/Components/Component.h"

namespace LevelBossStates
{
	class BossCluster : public Component
	{
		COMPONENT(BossCluster, "Boss Cluster");
	public:
		BossCluster() = default;
		~BossCluster();
		
		void Awake() override;
		void Reflect(Engine::Reflector& aReflector) override;

		void Execute(Engine::eEngineOrder aOrder) override;

		void SetPositions(const Vec3f& anStartPosition, const Vec3f& anEndPosition, const float aHeight, const float aDuration, const float aProgress, const float aStartRatio);

		void SetClusterRadius(const float aRadius);
		void SetDamage(const float anAmount);
		
	private:
		void Explode();

	private:
		Engine::AudioComponent* myAudioComponent = nullptr;
		VFXRef myExplosionVFXRef;

		GameObjectPrefabRef myTelegraphPrefab;
		GameObject* myTelegraphObject = nullptr;

		Vec3f myStartPosition = {};
		Vec3f myEndPosition = {};
		Vec3f myLastFramePosition = {};
		float myHeight = 0.0f;

		float myDuration = 0.0f;
		float myProgress = 0.0f;
		
		float myDamage = 10.0f;
		float myClusterRadius = 100.f;

		float myStartRatio = 0.f;
	};
}
namespace LBS = LevelBossStates;