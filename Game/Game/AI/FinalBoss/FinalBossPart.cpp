#include "pch.h"
#include "FinalBossPart.h"
#include "Engine/GameObject/GameObject.h"
#include "Components/HealthComponent.h"
#include "Engine/GameObject/Components/RigidBodyComponent.h"
#include "Engine/GameObject/Components/AnimatorComponent.h"
#include "Engine/GameObject/Components/ModelComponent.h"
#include "FinalBoss.h"

FB::FinalBossPart::FinalBossPart()
{
}

FB::FinalBossPart::FinalBossPart(GameObject* aGameObject)
	: Component(aGameObject)
{
}

FB::FinalBossPart::~FinalBossPart()
{
}

void FB::FinalBossPart::Awake()
{
	if (!myColliderGameObject)
	{
		LOG_ERROR(LogType::Game) << "Hand is missing reference to its collider";
		myGameObject->Destroy();
		return;
	}

	if (!myModelGameObject)
	{
		LOG_ERROR(LogType::Game) << "Hand is missing reference to its model";
		myGameObject->Destroy();
		return;
	}

	if (!myFinalBossGameObject)
	{
		LOG_ERROR(LogType::Game) << "Hand is missing reference to its model";
		myGameObject->Destroy();
		return;
	}

	myAnimator = myGameObject->FindComponent<Engine::AnimatorComponent>();
	myModel = myModelGameObject.Get()->GetComponent<Engine::ModelComponent>();
	myFinalBoss = myFinalBossGameObject.Get()->GetComponent<FinalBoss>();

	myHealth = myGameObject->GetComponent<HealthComponent>();
	myHealth->ObserveDamage([this](float aDamage) { OnDamaged(aDamage); });
	myHealth->ObserveDeath([this](float _) { OnDeath(); });
	myHealth->SetIsInvincible(true);
}

void FB::FinalBossPart::Start()
{
	//auto collider = myColliderGameObject.Get()->GetComponent<Engine::RigidBodyComponent>();

	//collider->ObserveTriggerEnter([this](GameObject* g)
	//	{
	//		if (g->GetTag() == eTag::PLAYER)
	//		{
	//			if (auto health = g->GetComponent<HealthComponent>())
	//			{
	//				const float damage = Random::RandomFloat(1, 2);
	//				health->ApplyDamage(damage);
	//				// health->ApplyKnockback(Vec3f(0, 1, 0) * myHandPunchDesc.myHitForce);
	//			}
	//		}
	//	});
}

void FB::FinalBossPart::Reflect(Engine::Reflector& aReflector)
{
	Component::Reflect(aReflector);

	aReflector.Header("Part");

	aReflector.Reflect(myColliderGameObject, "Collider");
	aReflector.Reflect(myModelGameObject, "Model");
	aReflector.Reflect(myFinalBossGameObject, "Final Boss");

	aReflector.Reflect(myDamagedColor, "Damaged Color", Engine::ReflectionFlags_IsColor);
	aReflector.Reflect(myDoNextBehaviourCooldownRange, "Behaviour Cooldown Range");

	if (myActiveBehaviour)
	{
		myActiveBehaviour->Reflect(aReflector);
	}

	aReflector.Reflect(myNextBehaviourDownTimer, "NextBehaviourDownTimer", Engine::ReflectionFlags_ReadOnly);

	aReflector.Reflect(myMaxEmissionGlow, "Max Emission Glow");
}

bool FB::FinalBossPart::IsDead() const
{
	return myHealth->GetHealth() <= 0;
}

bool FB::FinalBossPart::IsDemolished() const
{
	LOG_ERROR(LogType::Game) << "Unimplemented FinalBossPart::IsDemolished()";
	return false;
}

Engine::AnimatorComponent& FB::FinalBossPart::GetAnimator()
{
	return *myAnimator;
}

HealthComponent& FB::FinalBossPart::GetHealth()
{
	return *myHealth;
}

FB::FinalBoss& FB::FinalBossPart::GetFinalBoss()
{
	return *myFinalBoss;
}

FB::BehaviourEnumValue FB::FinalBossPart::GetBehaviour() const
{
	return myActiveBehaviourValue;
}

FB::Behaviour* FB::FinalBossPart::GetActiveBehaviour() const
{
	return myActiveBehaviour;
}

GameObject* FB::FinalBossPart::GetColliderGameObject()
{
	if (myColliderGameObject && myColliderGameObject.IsValid())
	{
		return myColliderGameObject.Get();
	}
	return nullptr;
}

GameObject* FB::FinalBossPart::GetModelGameObject()
{
	return myModelGameObject.Get();
}

void FB::FinalBossPart::AddNormalBehaviour(const BehaviourEnumValue aBehaviourVal, Owned<Behaviour> aBehaviour)
{
	const int index = myNormalBehaviours.size();
	myNormalBehaviours[aBehaviourVal] = std::move(aBehaviour);
}

void FB::FinalBossPart::AddAttackBehaviour(const BehaviourEnumValue aBehaviourVal, Owned<Behaviour> aBehaviour)
{
	const int index = myAttackBehaviours.size();
	myAttackBehaviours[aBehaviourVal] = std::move(aBehaviour);
}

FB::Behaviour* FB::FinalBossPart::GetNormalBehaviour(const BehaviourEnumValue aBehaviourVal) const
{
	auto f = myNormalBehaviours.find(aBehaviourVal);

	if (f != myNormalBehaviours.end())
	{
		return f->second.get();
	}

	return nullptr;
}

FB::Behaviour* FB::FinalBossPart::GetAttackBehaviour(const BehaviourEnumValue aBehaviourVal) const
{
	auto f = myAttackBehaviours.find(aBehaviourVal);

	if (f != myAttackBehaviours.end())
	{
		return f->second.get();
	}

	return nullptr;
}

void FB::FinalBossPart::ChangeBehaviour(const BehaviourEnumValue aBehaviour)
{
	if (myActiveBehaviour)
	{
		// If exiting attack behaviour
		if (IsAttackBehaviour(myActiveBehaviour))
		{
			SetEmissionGlowTarget(0.f);
		}

		myActiveBehaviour->OnExit();
	}

	Behaviour* choosenBehaviour = nullptr;
	auto f1 = myNormalBehaviours.find(aBehaviour);

	if (f1 != myNormalBehaviours.end())
	{
		choosenBehaviour = f1->second.get();
	}

	auto f2 = myAttackBehaviours.find(aBehaviour);
	if (f2 != myAttackBehaviours.end())
	{
		choosenBehaviour = f2->second.get();
	}

	myActiveBehaviour = choosenBehaviour;
	myActiveBehaviourValue = aBehaviour;

	if (myActiveBehaviour)
	{
		myActiveBehaviour->OnEnter();

		// If changing to attack behaviour
		if (IsAttackBehaviour(myActiveBehaviour))
		{
			SetEmissionGlowTarget(myMaxEmissionGlow);
		}
	}
}

void FB::FinalBossPart::SetBaseBehaviour(const BehaviourEnumValue aBehaviour)
{
	myBaseBehaviour = aBehaviour;
}

void FB::FinalBossPart::UpdateActiveBehaviour()
{
	if (myActiveBehaviour)
	{
		myActiveBehaviour->Update();

		if (myActiveBehaviour->IsFinished())
		{
			ChangeBehaviour(myBaseBehaviour);
		}
	}
}

void FB::FinalBossPart::UpdateDamagedColor()
{
	myDamagedTimer.Tick(Time::DeltaTime);

	const Vec3f defaultColor(1.f, 1.f, 1.f);

	if (myDamagedTimer.IsFinished())
	{
		// myModel->GetMeshMaterialInstanceByIndex(0).SetMainColor(defaultColor.ToVec4(1.f));

		// Turn off glow
		myMaterialValues.z = myGlowMinMax.x;
	}
	else
	{
		// const Vec3f color = Math::Lerp(myDamagedColor, defaultColor, myDamagedTimer.Percent());
		// myModel->GetMeshMaterialInstanceByIndex(0).SetMainColor(color.ToVec4(1.f));

		const float glow = Math::Lerp(myGlowMinMax.y, myGlowMinMax.x, myDamagedTimer.Percent());

		// Turn on glow
		myMaterialValues.z = glow;
	}

	myModel->GetMeshMaterialInstanceByIndex(0).SetFloat4("emissiveStr", myMaterialValues);
}

void FB::FinalBossPart::UpdateEmissionGlow()
{
	const float emissionGlowSpeed = 4.f;
	myCurrentEmissionGlow = Math::Lerp(
		myCurrentEmissionGlow,
		myEmissionGlowTarget,
		emissionGlowSpeed * Time::DeltaTime);

	myMaterialValues.x = myCurrentEmissionGlow;

	myModel->GetMeshMaterialInstanceByIndex(0).SetFloat4("emissiveStr", myMaterialValues);
}

void FB::FinalBossPart::OnDamaged(const float aDamage)
{
	myDamagedTimer = TickTimer::FromSeconds(0.3f);
}

void FB::FinalBossPart::UpdateAttackTransitions()
{
	if (myActiveBehaviour == nullptr ||
		// !IsAttackBehaviour(myActiveBehaviourValue) ||
		myActiveBehaviour->IsAllowedToInterrupt())
	{
		myNextBehaviourDownTimer -= Time::DeltaTime;
		myNextBehaviourDownTimer = std::max(myNextBehaviourDownTimer, 0.f);

		if (myNextBehaviourDownTimer <= 0.f)
		{
			myNextBehaviourDownTimer = Random::RandomFloat(
				myDoNextBehaviourCooldownRange.x,
				myDoNextBehaviourCooldownRange.y);

			const auto attack = DetermineNextAttack();
			myLatestAttackEnumValue = attack;
			ChangeBehaviour(attack);
		}
	}
}

void FB::FinalBossPart::SetEmissionGlowTarget(const float aEmissionGlow)
{
	myEmissionGlowTarget = aEmissionGlow;
}

void FB::FinalBossPart::SetGlowMinMax(const Vec2f& aGlowMinMax)
{
	myGlowMinMax = aGlowMinMax;
}

void FB::FinalBossPart::RestockAttackBehaviourStack()
{
	if (myAttackBehaviourStack.empty())
	{
		std::vector<BehaviourEnumValue> temp;
		for (const auto& [behaviourEnumVal, _] : myAttackBehaviours)
		{
			temp.push_back(behaviourEnumVal);
		}

		const int size = temp.size();

		for (int i = 0; i < size; ++i)
		{
			const int index = Random::RandomInt(0, temp.size() - 1);

			myAttackBehaviourStack.push_back(temp[index]);

			temp.erase(temp.begin() + index);
		}

		// Ensure 1 attack does not occur 2 times in a row
		if (myAttackBehaviourStack.back() == myLatestAttackEnumValue)
		{
			// Must move it to bottom in the list
			std::swap(myAttackBehaviourStack.front(), myAttackBehaviourStack.back());
		}
	}
}

FB::BehaviourEnumValue FB::FinalBossPart::DetermineNextAttack()
{
	auto r = myAttackBehaviourStack.back();
	myAttackBehaviourStack.pop_back();
	return r;
}

bool FB::FinalBossPart::IsAttackBehaviour(const BehaviourEnumValue aBehaviourEnumVal) const
{
	return myAttackBehaviours.find(aBehaviourEnumVal) != myAttackBehaviours.end();
}

bool FB::FinalBossPart::IsAttackBehaviour(const Behaviour* aBehaviour) const
{
	for (const auto& [enumVal, behaviour] : myAttackBehaviours)
	{
		if (behaviour.get() == aBehaviour)
		{
			return true;
		}
	}

	return false;
}
