#pragma once

#include "Engine/GameObject/Components/Component.h"
#include "Engine/GameObject/GameObjectRef.h"
#include "Behaviours/HandCrawler.h"
#include "Utils/TickTimer.h"
#include "Game/AI/FinalBoss/FinalBossPhase.h"
#include <stack>

namespace FB
{
	class Behaviour;
	class FinalBoss;
	class AnimationController;

	class FinalBossPart : public Component
	{
	public:
		FinalBossPart();
		FinalBossPart(GameObject* aGameObject);
		virtual ~FinalBossPart();

		virtual void OnEnterPhase(const FinalBossPhase aPhase) {}

		bool IsDead() const;

		// If has been physically destroyed
		virtual bool IsDemolished() const;

		Engine::AnimatorComponent& GetAnimator();
		HealthComponent& GetHealth();
		FinalBoss& GetFinalBoss();

		BehaviourEnumValue GetBehaviour() const;
		Behaviour* GetActiveBehaviour() const;

		virtual bool IsIntroFinished() const { return true; }

		GameObject* GetColliderGameObject();

		GameObject* GetModelGameObject();

	protected:
		virtual void Awake() override;
		virtual void Start() override;
		virtual void Reflect(Engine::Reflector& aReflector) override;

		void AddNormalBehaviour(const BehaviourEnumValue aBehaviourVal, Owned<Behaviour> aBehaviour);
		void AddAttackBehaviour(const BehaviourEnumValue aBehaviourVal, Owned<Behaviour> aBehaviour);
		Behaviour* GetNormalBehaviour(const BehaviourEnumValue aBehaviourVal) const;
		Behaviour* GetAttackBehaviour(const BehaviourEnumValue aBehaviourVal) const;

		void ChangeBehaviour(const BehaviourEnumValue aBehaviour);

		void SetBaseBehaviour(const BehaviourEnumValue aBehaviour);

		void RestockAttackBehaviourStack();
		void UpdateActiveBehaviour();
		void UpdateDamagedColor();
		void UpdateEmissionGlow();
		void UpdateAttackTransitions();

		void SetEmissionGlowTarget(const float aEmissionGlow);
		void SetGlowMinMax(const Vec2f& aGlowMinMax);

	private:
		virtual void OnDamaged(const float aDamage);
		virtual void OnDeath() {}

		BehaviourEnumValue DetermineNextAttack();
		bool IsAttackBehaviour(const BehaviourEnumValue aBehaviourEnumVal) const;
		bool IsAttackBehaviour(const Behaviour* aBehaviour) const;

	protected:
		Engine::AnimatorComponent* myAnimator = nullptr;
		Engine::ModelComponent* myModel = nullptr;
		HealthComponent* myHealth = nullptr;
		FinalBoss* myFinalBoss = nullptr;

		Engine::GameObjectRef myColliderGameObject;
		Engine::GameObjectRef myModelGameObject;
		Engine::GameObjectRef myFinalBossGameObject;

		std::unordered_map<BehaviourEnumValue, Owned<Behaviour>> myNormalBehaviours;
		std::unordered_map<BehaviourEnumValue, Owned<Behaviour>> myAttackBehaviours;
		Behaviour* myActiveBehaviour = nullptr;
		BehaviourEnumValue myActiveBehaviourValue = 0;

		BehaviourEnumValue myBaseBehaviour = 0;

		Vec2f myDoNextBehaviourCooldownRange = Vec2f(0.2f, 1.f);
		float myNextBehaviourDownTimer = 0.f;

		Vec3f myDamagedColor = Vec3f(1.f, 0.f, 0.f);
		TickTimer myDamagedTimer;
		float myCurrentEmissionGlow = 0.f;
		float myEmissionGlowTarget = 0.f;

		//std::stack<BehaviourEnumValue> myAttackBehaviourStack;
		std::vector<BehaviourEnumValue> myAttackBehaviourStack;

		Vec4f myMaterialValues;

		Vec2f myGlowMinMax = Vec2f(0.f, 0.1f);

		float myMaxEmissionGlow = 10.f;

		BehaviourEnumValue myLatestAttackEnumValue = -1;
	};
}