#include "pch.h"
#include "HealthComponent.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/Reflection/Reflector.h"
#include "Prototype/DamageNumber.h"
#include "Prototype/FloatingText.h"

#include "Engine/Utils/TimerManager.h"

#include "Engine/ResourceManagement/Resources/VFXResource.h"
#include "Engine/GameObject/Components/VFXComponent.h"
#include "Engine/GameObject/Components/ModelComponent.h"
#include "Player/Player.h"

constexpr float invulnerableTimerDuration = .5f;
const CU::Vector4f invulnerableColor( 100.f, 1000.f, 1000.f, 0.f );

HealthComponent::HealthComponent(GameObject* aGameObject, const float aMaxHealth)
	: Component(aGameObject)
{
	//assert(aMaxHealth > 0 && "WARNING: You have not set a MaxHP in AddComponent<HealthComponent> OR it was negative");

	myHealth.baseMaxHp = aMaxHealth;
	myHealth.maxHP = aMaxHealth;
	myHealth.hp = aMaxHealth;
}

HealthComponent::~HealthComponent()
{
	if (Main::Exists()) 
	{
		Main::GetTimerManager().StopTimer(myInvulnerability.timerIndex);
	}
}

void HealthComponent::Start()
{
	if (myShieldVFXFRef && myShieldVFXFRef->IsValid())
	{
		myShieldVFX = myGameObject->AddComponent<Engine::VFXComponent>(myShieldVFXFRef->Get());
	}
}

void HealthComponent::Reflect(Engine::Reflector& aReflector)
{
	Component::Reflect(aReflector);

	aReflector.Reflect(myShouldOneShotProtect, "One Shot Protection");

	aReflector.Header("Health");
	if (aReflector.Reflect(myHealth.baseMaxHp, "Max Health") & Engine::ReflectorResult_Changed)
	{
		myHealth.maxHP = myHealth.baseMaxHp * myHealth.maxHpMod;
		myHealth.hp = myHealth.maxHP;
	}
	aReflector.Reflect(myHealth.hp, "Health", Engine::ReflectionFlags_ReadOnly);
	aReflector.Reflect(myHealth.maxHP, "Modified Max HP", Engine::ReflectionFlags_ReadOnly);
	aReflector.Reflect(myHealth.maxHpMod, "Max HP modifier", Engine::ReflectionFlags_ReadOnly);
	aReflector.Reflect(myHealth.shouldRegenHealth, "Regen Health");

	aReflector.Header("Shield");
	aReflector.Reflect(myShieldVFXFRef, "Shield VFX");
	aReflector.Reflect(myShield.maxShield, "Max shield", Engine::ReflectionFlags_ReadOnly);
	aReflector.Reflect(myShield.shield, "Current shield", Engine::ReflectionFlags_ReadOnly);
	aReflector.Reflect(myBarrier.barrier, "Barrier", Engine::ReflectionFlags_ReadOnly);
}

void HealthComponent::Execute(Engine::eEngineOrder aOrder)
{
	UpdateBuffs();

	if (myHealth.shouldRegenHealth)
		UpdateHealthRegen();

	UpdateShield();
	UpdateBarrierDecay();
	UpdateInvincibility();
}

void HealthComponent::ApplyDamage(const float aAmount, bool aIgnoreShield, const CU::Vector3f* anImpactPosition, eDamageType aDamageType)
{
	assert(aAmount > -0.00001 && "Damage cannot be negative!");

	if (myIsInShield)
	{
		return;
	}

	if (myHealth.hp <= 0.0f || aAmount <= 0.0f)
		return;

	if (Dodged())
	{
		SpawnFloatingText("DODGE", anImpactPosition);
		for (auto& observer : myDodgeObserver)
		{
			if (!observer)
				continue;

			observer();
		}
		return;
	}

	myHealth.bufferTimer = 0.0f;
	myShield.bufferTimer = 0.0f;

	float remainingAmount = aAmount * myHealth.damageModifier;

	if (myShouldOneShotProtect && remainingAmount > GetTotalMaxHP() * 0.9f) //One Shot Protection
	{
		remainingAmount = GetTotalMaxHP() * 0.9f;
	}


	if (!aIgnoreShield)
	{
		const float barrierDamage = CU::Min(remainingAmount, myBarrier.barrier);
		myBarrier.barrier -= barrierDamage;
		remainingAmount -= barrierDamage;

		if (myBarrierChangeObserver)
			myBarrierChangeObserver(myBarrier.barrier);
	}

	if (!aIgnoreShield)
	{
		const float shieldDamage = CU::Min(remainingAmount, myShield.shield);
		myShield.shield -= shieldDamage;
		remainingAmount -= shieldDamage;

		if (myShieldChangeObserver)
			myShieldChangeObserver(myShield.shield, myShield.maxShield);
	}

	if (myInvulnerability.isInvulnerable)
	{
		SpawnFloatingText("Invulnerable!", anImpactPosition);
		return;
	}

	if (remainingAmount >= myHealth.hp)
	{
		remainingAmount -= myHealth.hp;
		myHealth.hp = 0;
		for (auto& observer : myDeathObserver)
		{
			if (!observer)
				continue;

			observer(remainingAmount);
		}
	}
	else
	{
		myHealth.hp -= remainingAmount;
		remainingAmount = 0.f;

		for (auto& observer : myApplyDamageObserver)
		{
			if (!observer)
				continue;

			observer(remainingAmount);
		}
	}

	SpawnDamageNumbers(aAmount * myHealth.damageModifier - remainingAmount, anImpactPosition, aDamageType);
}

void HealthComponent::ApplyKnockback(const Vec3f& aDirection)
{
	if (myApplyKnockObserver)
		myApplyKnockObserver(aDirection);
	// Maybe add some kind of decided behaviour
}

void HealthComponent::ApplySlow(float aSlowPercentage, float aSlowTime)
{
	// TODO: Later add effect list and choose strongest one
	AddSlow(aSlowPercentage, aSlowTime);

	if (myApplySlowObserver)
		myApplySlowObserver(aSlowPercentage, aSlowTime);
}

void HealthComponent::ApplyStun(float aStunTime)
{
	myStun.time = aStunTime;

	if (myApplyStunObserver)
	{
		myApplyStunObserver(aStunTime);
	}
}

bool HealthComponent::Dodged()
{
	auto rnd = Random::RandomFloat(0, 1);
	if (rnd < myDodgeChance)
	{
		for (auto& observer : myDodgeObserver)
		{
			if (!observer)
				continue;

			observer();
		}
		return true;
	}
	return false;
}

float& HealthComponent::GetDodgeChance()
{
	return myDodgeChance;
}

void HealthComponent::SetDodgeChance(float aValue)
{
	myDodgeChance = aValue;
	myDodgeChance = CU::Clamp(0.f, 0.95f, myDodgeChance);
}

void HealthComponent::AddDodgeChance(float aValue)
{
	myDodgeChance += aValue;
	myDodgeChance = CU::Clamp(0.f, 0.95f, myDodgeChance);
}

void HealthComponent::AddShield(float aShieldAmount)
{
	myShield.shield += aShieldAmount;
	myShield.shield = std::clamp(myShield.shield, 0.0f, myShield.maxShield);

	if (myShieldChangeObserver)
		myShieldChangeObserver(myShield.shield, myShield.maxShield);
}

void HealthComponent::RemoveShield(float aShieldAmount)
{
	if (myShield.shield - aShieldAmount < myShield.maxShield)
	{
		myShield.shield = CU::Min(myShield.shield, myShield.maxShield);
	}
	else
	{
		myShield.shield -= aShieldAmount;
	}

	if (myShieldChangeObserver)
		myShieldChangeObserver(myShield.shield, myShield.maxShield);
}

void HealthComponent::AddPermanentShield(float aPermanentShield)
{
	myShield.maxShield += aPermanentShield;
	myShield.shield = myShield.maxShield;

	if (myShieldChangeObserver)
		myShieldChangeObserver(myShield.shield, myShield.maxShield);
}

float HealthComponent::GetShield() const
{
	return myShield.shield;
}

float HealthComponent::GetMaxShield() const
{
	return myShield.maxShield;
}

void HealthComponent::AddBarrier(float aBarrierAmount)
{
	myBarrier.barrier += aBarrierAmount;
	myBarrier.barrier = std::min(myBarrier.barrier, myHealth.maxHP + myShield.maxShield);
	//The maximum amount of barrier obtainable is equal to the player's maximum health plus their maximum shields

	if (myBarrierChangeObserver)
		myBarrierChangeObserver(myBarrier.barrier);
}

void HealthComponent::RemoveBarrier(float aBarrierAmount)
{
	myBarrier.barrier -= aBarrierAmount;
	myBarrier.barrier = std::max(myBarrier.barrier, 0.0f);

	if (myBarrierChangeObserver)
		myBarrierChangeObserver(myBarrier.barrier);
}

float HealthComponent::GetBarrier() const
{
	return myBarrier.barrier;
}

float HealthComponent::GetTotalHP() const
{
	return myHealth.hp + myShield.shield + myBarrier.barrier;
}

float HealthComponent::GetTotalMaxHP() const
{
	return myHealth.maxHP + myShield.maxShield + myBarrier.barrier;
}

void HealthComponent::SetBaseArmor(float aBase)
{
	myArmor.base = aBase;
}

float HealthComponent::GetArmor() const
{
	return myArmor.base + myArmor.modifier;
}

float HealthComponent::GetArmorBase() const
{
	return myArmor.base;
}

void HealthComponent::ObserveKnockback(std::function<void(const Vec3f&)> aObserveFunction)
{
	myApplyKnockObserver = aObserveFunction;
}

void HealthComponent::ObserveStun(std::function<void(float)> aObserveFunction)
{
	myApplyStunObserver = aObserveFunction;
}

void HealthComponent::ObserveSlow(std::function<void(float, float)> aObserveFunction)
{
	myApplySlowObserver = aObserveFunction;
}

void HealthComponent::ObserveShield(std::function<void(float, float)> aObserveFunction)
{
	myShieldChangeObserver = aObserveFunction;
}

void HealthComponent::ObserveBarrier(std::function<void(float)> aObserveFunction)
{
	myBarrierChangeObserver = aObserveFunction;
}

void HealthComponent::AddSlow(float aSlowPercentage, float aSlowTime)
{
	mySlow.push_back(Slow(aSlowPercentage, aSlowTime));
}

float HealthComponent::GetSlowPercentage() const
{
	int index = 0;

	if (mySlow.size() > 1)
	{
		for (int slowIndex = 0; slowIndex < mySlow.size(); slowIndex++)
		{
			if (mySlow[slowIndex].slowPercentage > mySlow[index].slowPercentage)
			{
				index = slowIndex;
			}
		}
		return mySlow[index].slowPercentage;
	}
	else if (mySlow.size() == 1)
	{
		return mySlow[0].slowPercentage;
	}
	else
	{
		return 0.f;
	}
}

float HealthComponent::GetSlowTime() const
{
	int index = 0;

	if (mySlow.size() > 1)
	{
		for (int slowIndex = 0; slowIndex < mySlow.size(); slowIndex++)
		{
			if (mySlow[slowIndex].slowPercentage > mySlow[index].slowPercentage)
			{
				index = slowIndex;
			}
		}
		return mySlow[index].slowTime;
	}
	else
	{
		return mySlow[0].slowTime;
	}
}

bool HealthComponent::UpdateSlowTimers()
{
	for (int slowIndex = 0; slowIndex < mySlow.size(); slowIndex++)
	{
		mySlow[slowIndex].slowTimer -= Time::DeltaTime;
		if (mySlow[slowIndex].slowTimer <= 0)
		{
			mySlow.erase(mySlow.begin() + slowIndex);
		}
	}

	if (mySlow.size() == 0)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void HealthComponent::ApplyHealing(const float aAmount)
{
	if (myHealth.hp + aAmount >= myHealth.maxHP)
	{
		myHealth.hp = myHealth.maxHP;
		if (myApplyHealingObserver)
		{
			myApplyHealingObserver(aAmount);
		}
	}
	else if (myHealth.hp + aAmount < 1.f)
	{
		myHealth.hp = 1.f;
	}
	else
	{
		myHealth.hp += aAmount;
		if (myApplyHealingObserver)
		{
			myApplyHealingObserver(aAmount);
		}
	}
}

void HealthComponent::AddMaxHealth(const float aAmount)
{
	myHealth.baseMaxHp += aAmount;
	myHealth.maxHP = myHealth.baseMaxHp * myHealth.maxHpMod;
	myHealth.hp += aAmount * myHealth.maxHpMod;
	if (myHealth.hp > myHealth.maxHP)
	{
		myHealth.hp = myHealth.maxHP;
	}
	if (myHealth.hp < 1.f)
	{
		myHealth.hp = 1.f;
	}

	if (myApplyHealingObserver)
	{
		myApplyHealingObserver(aAmount);
	}
}

void HealthComponent::AddMaxHealthPercent(float aModifier)
{
	float oldMaxHP = myHealth.maxHP;
	myHealth.maxHpMod += aModifier;
	myHealth.maxHP = myHealth.baseMaxHp * myHealth.maxHpMod;
	myHealth.hp += (myHealth.maxHP - oldMaxHP);
	if (myHealth.hp > myHealth.maxHP)
	{
		myHealth.hp = myHealth.maxHP;
	}
	if (myHealth.hp < 1.f)
	{
		myHealth.hp = 1.f;
	}

	if (myApplyHealingObserver)
	{
		myApplyHealingObserver((myHealth.maxHP - oldMaxHP));
	}
}

void HealthComponent::SetMaxHealthModifier(float aModifier)
{
	float oldMaxHP = myHealth.maxHP;
	myHealth.maxHpMod = aModifier;
	myHealth.maxHP = myHealth.baseMaxHp * myHealth.maxHpMod;
	myHealth.hp == myHealth.maxHP;
	if (myApplyHealingObserver)
	{
		myApplyHealingObserver((myHealth.maxHP - oldMaxHP));
	}
}

void HealthComponent::SetMaxHealth(const float aMaxHealth)
{
	float oldMaxHP = myHealth.maxHP;
	myHealth.baseMaxHp = aMaxHealth;
	myHealth.maxHP = myHealth.baseMaxHp * myHealth.maxHpMod;
	myHealth.hp = myHealth.maxHP;

	if (myApplyHealingObserver)
	{
		myApplyHealingObserver((myHealth.maxHP - oldMaxHP));
	}
}

void HealthComponent::SetHealth(const float aHealth)
{
	myHealth.hp = aHealth;
}

void HealthComponent::ResetHealthToMax()
{
	myHealth.hp = myHealth.maxHP;
}

void HealthComponent::SetDamageModifier(float aModifier)
{
	myHealth.damageModifier = aModifier;
}

float HealthComponent::GetDamageModifier() const
{
	return myHealth.damageModifier;
}

void HealthComponent::ObserveDamage(std::function<void(float)> aFunction)
{
	myApplyDamageObserver.push_back(aFunction);
}

void HealthComponent::ObserveDodge(std::function<void()> aFunction)
{
	myDodgeObserver.push_back(aFunction);
}

void HealthComponent::ObserveDeath(std::function<void(float)> aFunction)
{
	myDeathObserver.push_back(aFunction);
}

void HealthComponent::ObserveHealing(std::function<void(float)> aFunction)
{
	myApplyHealingObserver = aFunction;
}

float HealthComponent::GetMaxHealth() const
{
	return myHealth.maxHP;
}

float HealthComponent::GetHealth() const
{
	return myHealth.hp;
}

float HealthComponent::GetMaxHealthBase() const
{
	return myHealth.baseMaxHp;
}

float HealthComponent::GetMaxHealthMod() const
{
	return myHealth.maxHpMod;
}

void HealthComponent::SetIsInvincible(const bool aIsInvincible)
{
	if (!Main::Exists()) 
	{
		return;
	}

	if (aIsInvincible)
	{
		myInvulnerability.isInvulnerable = true;
	}

	Main::GetTimerManager().StopTimer(myInvulnerability.timerIndex);
	if (aIsInvincible)
	{
		myInvulnerability.timerIndex = Main::GetTimerManager().SetTimer(invulnerableTimerDuration, [this] {FinishSwitchingInvulnerability(true); });
	}
	else
	{
		myInvulnerability.timerIndex = Main::GetTimerManager().SetTimer(invulnerableTimerDuration, [this] {FinishSwitchingInvulnerability(false); });
	}

	myInvulnerability.targetInvulnerability = aIsInvincible;
	myInvulnerability.inTransition = true;
}

void HealthComponent::SetIsInShield(bool aIsInShield)
{
	myIsInShield = aIsInShield;
}

bool HealthComponent::HasFullHealth() const
{
	return myHealth.hp >= myHealth.maxHP;
}

bool HealthComponent::HasNoHealth() const
{
	return myHealth.hp <= 0.0f;
}

float& HealthComponent::GetHealthRef()
{
	return myHealth.hp;
}

void HealthComponent::ApplyBuff(std::unique_ptr<Buff> aBuff, int aMaxStacks)
{
	if (aMaxStacks > 1)
	{
		auto it = std::find(myBuffs.begin(), myBuffs.end(), aBuff->GetID());

		if (it != myBuffs.end())
		{
			if (it->second->GetStacks() < aMaxStacks)
			{
				it->second->AddStack();
			}
			it->second->Refresh(aBuff->GetDuration());

			return;
		}
	}

	myBuffs.push_back(std::make_pair(aBuff->GetID(), std::move(aBuff)));
}

Buff* HealthComponent::GetBuff(BuffID anID)
{
	auto it = std::find(myBuffs.begin(), myBuffs.end(), anID);
	if (it != myBuffs.end())
	{
		return it->second.get();
	}
	return nullptr;
}

void HealthComponent::ClearBuffs()
{
	myBuffs.clear();
}

void HealthComponent::UpdateBuffs()
{
	std::vector<size_t> indicesToDelete;

	for (size_t i = 0; i < myBuffs.size(); ++i)
	{
		if (myBuffs[i].second->Update(Time::DeltaTime))
		{
			indicesToDelete.push_back(i);
		}
	}

	for (int i = static_cast<int>(indicesToDelete.size()) - 1; i >= 0; --i)
	{
		myBuffs.erase(myBuffs.begin() + indicesToDelete[i]);
	}
}
void HealthComponent::UpdateInvincibility()
{
	if (myInvulnerability.inTransition)
	{
		float timerRemainingTime = Main::GetTimerManager().CheckTimer(myInvulnerability.timerIndex);
		float multiplier = 0;
		if (myInvulnerability.targetInvulnerability)
		{
			multiplier = (invulnerableTimerDuration - timerRemainingTime) / invulnerableTimerDuration;
		}
		else
		{
			multiplier = timerRemainingTime / invulnerableTimerDuration;
		}


		GameObject* modelObject = Main::GetPlayer()->GetTransform().GetChildByGameObjectName("Model");
		if (!modelObject)
			return;
		Engine::ModelComponent* model = modelObject->GetComponent<Engine::ModelComponent>();
		if (!model)
			return;
		model->GetMeshMaterialInstanceByIndex(0).SetFloat4("pkBless", invulnerableColor *multiplier);
	}
}

void HealthComponent::FinishSwitchingInvulnerability(bool aInvulnerable)
{
	myInvulnerability.inTransition = false;
	GameObject* modelObject = Main::GetPlayer()->GetTransform().GetChildByGameObjectName("Model");
	if (!modelObject)
		return;
	Engine::ModelComponent* model = modelObject->GetComponent<Engine::ModelComponent>();
	if (!model)
		return;
	myInvulnerability.isInvulnerable = aInvulnerable;
	model->GetMeshMaterialInstanceByIndex(0).SetFloat4("pkBless", invulnerableColor * (myInvulnerability.isInvulnerable ? 1.f : 0.f));
}

void HealthComponent::UpdateHealthRegen()
{
	if (myHealth.hp <= 0.0f)
		return;

	if (myHealth.hp == myHealth.maxHP)
		return;

	myHealth.bufferTimer += Time::DeltaTime;

	if (myHealth.bufferTimer > myHealth.bufferTime)
	{
		float restoredHealth = myHealth.restoreSpeed * Time::DeltaTime;

		ApplyHealing(restoredHealth);
	}
}

void HealthComponent::UpdateBarrierDecay()
{
	if (myHealth.hp <= 0.0f)
		return;

	if (myBarrier.barrier <= 0.0f)
		return;

	//Barrier degrades rapidly over time at a rate of 3.33% of the player's maximum health per second.
	float decay = (GetTotalMaxHP() - myBarrier.barrier) * 0.033f * Time::DeltaTime;

	RemoveBarrier(decay);
}

void HealthComponent::UpdateShield()
{
	if (myHealth.hp <= 0.0f)
		return;

	if (myShield.maxShield == 0.0f)
		return;

	if (myShieldVFX)
	{
		if (myShield.shield == 0 && myShieldVFX->IsPlaying())
		{
			myShieldVFX->Stop();
		}
		else if (myShield.shield != 0 && !myShieldVFX->IsPlaying())
		{
			myShieldVFX->Play();
		}
	}


	if (myShield.shield == myShield.maxShield)
		return;

	myShield.bufferTimer += Time::DeltaTime;

	if (myShield.bufferTimer > myShield.bufferTime)
	{
		float restoredShield = myShield.restoreSpeed * Time::DeltaTime;

		AddShield(restoredShield);
	}
}

void HealthComponent::SpawnDamageNumbers(float aDamage, const CU::Vector3f* aPosition, eDamageType aDamageType)
{
	//TODO: different colors for different types of damage? Differentiate between ticks and impact`?

	CU::Vector3f pos;

	if (aPosition)
	{
		pos = *aPosition;
		CU::Vector3f playerDir = (Main::GetPlayer()->GetTransform().GetPosition() - GetTransform().GetPosition()).GetNormalized();
		pos = pos + playerDir * 30.f;
	}
	else
	{
		CU::Vector3f playerDir = (Main::GetPlayer()->GetTransform().GetPosition() - GetTransform().GetPosition()).GetNormalized();
		pos = GetTransform().GetPosition() + playerDir * 100.f + CU::Vector3f{ 0.f, 70.f, 0.f };
	}
	GameObject* obj = myGameObject->GetScene()->AddGameObject<GameObject>();
	DamageNumber* dmgNumber = obj->AddComponent<DamageNumber>(aDamage);
	dmgNumber->SetPreset(aDamageType);
	obj->GetTransform().SetPosition(pos);
}

void HealthComponent::SpawnFloatingText(std::string aText, const CU::Vector3f* aPosition)
{
	//TODO: different colors for different types of damage? Differentiate between ticks and impact`?

	CU::Vector3f pos;

	if (aPosition)
	{
		pos = *aPosition;
		CU::Vector3f playerDir = (Main::GetPlayer()->GetTransform().GetPosition() - GetTransform().GetPosition()).GetNormalized();
		pos = pos + playerDir * 30.f;
	}
	else
	{
		CU::Vector3f playerDir = (Main::GetPlayer()->GetTransform().GetPosition() - GetTransform().GetPosition()).GetNormalized();
		pos = GetTransform().GetPosition() + playerDir * 100.f + CU::Vector3f{ 0.f, 70.f, 0.f };
	}
	GameObject* obj = myGameObject->GetScene()->AddGameObject<GameObject>();
	FloatingText* floatingText = obj->AddComponent<FloatingText>(aText);
	obj->GetTransform().SetPosition(pos);
}
