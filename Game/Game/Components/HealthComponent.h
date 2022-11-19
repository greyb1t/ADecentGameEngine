#pragma once
#include "Engine\GameObject\Components\Component.h"
#include "Items/Buff.h"

/* Stats
 * Health
 * Shield
 * Armor
 *
 * Effects
 * Knockback
 * Stun
 * Slow
 * Armor reduction
 * Overtime damage
 *
 */

 // Probably want to add effects that can be applied so we can procc other effects on them

enum class eDamageType
{
	Basic,
	Crit,
	Bleed, 
	Poison
};

class Buff;
struct BuffID;
class HealthComponent : public Component
{
public:
	COMPONENT(HealthComponent, "Health Component");

	HealthComponent() = default;
	HealthComponent(GameObject* aGameObject, const float aMaxHealth = -1);

	virtual ~HealthComponent();
	void Start() override;

	virtual void Reflect(Engine::Reflector& aReflector) override;

	virtual void Execute(Engine::eEngineOrder aOrder) override;

	virtual void	ApplyDamage(float aAmount, bool aIgnoreShield = false, const CU::Vector3f* anImpactPosition = nullptr, eDamageType aType = eDamageType::Basic);
	void	ApplyHealing(float aAmount);
	void	ApplyKnockback(const Vec3f& aDirection);
	void	ApplySlow(float aSlowPercentage, float aSlowTime);
	void	ApplyStun(float aStunTime);

	bool	Dodged();
	float& GetDodgeChance();
	void	SetDodgeChance(float aValue);
	void	AddDodgeChance(float aValue);

	/* HEALTH */
	void	SetHealth(const float aHealth);
	void	AddMaxHealth(const float aAmount);
	void	AddMaxHealthPercent(float aModifier);
	void	SetMaxHealthModifier(float aModifier);
	void	SetMaxHealth(const float aMaxHealth);
	void	ResetHealthToMax();

	void	SetDamageModifier(float aModifier);
	float	GetDamageModifier() const;

	float	GetMaxHealth() const;
	float	GetHealth() const;
	float	GetMaxHealthBase() const;
	float	GetMaxHealthMod() const;
	/* ------- */

	/* SHIELD */
	void	AddShield(float aShieldAmount);
	void	RemoveShield(float aShieldAmount);
	void	AddPermanentShield(float aPermanentShield);
	float	GetShield() const;
	float	GetMaxShield() const;
	/* ------- */

	/* BARRIER */
	void	AddBarrier(float aBarrierAmount);
	void	RemoveBarrier(float aBarrierAmount);
	float	GetBarrier() const;
	/* ------- */

		//Returns the players current total hit points
	float	GetTotalHP() const;
	//Returns the players maximum total hit points
	float	GetTotalMaxHP() const;

	/* ARMOR */
	void	SetBaseArmor(float aBase);
	float	GetArmor() const;
	float	GetArmorBase() const;
	/* ------- */


	/* OBSERVERS */
	void	ObserveDamage(std::function<void(float/*Damage*/)>);
	void	ObserveDodge(std::function<void()>);
	void	ObserveDeath(std::function<void(float/*Damage*/)> aFunction);
	void	ObserveHealing(std::function<void(float/*Healing*/)> aFunction);
	void	ObserveKnockback(std::function<void(const Vec3f& /*Knockback*/)>);
	void	ObserveStun(std::function<void(float /*Stun Time*/)>);
	void	ObserveSlow(std::function<void(float/*Slow Percentage*/, float /*Slow Time*/)>);
	void	ObserveShield(std::function<void(float/*Current Shield*/, float /*Max Shield*/)>);
	void	ObserveBarrier(std::function<void(float/*Barrier*/)>);
	/* --------- */

	/* SLOW */
	void	AddSlow(float aSlowPercentage, float aSlowTime);
	float	GetSlowPercentage() const;
	float	GetSlowTime() const;
	bool	UpdateSlowTimers();

	/* ------- */

	void	SetIsInvincible(bool aIsInvincible);

	void	SetIsInShield(bool aIsInShield);

	bool	HasFullHealth() const;
	bool	HasNoHealth() const;

	float& GetHealthRef();
	void  ApplyBuff(std::unique_ptr<Buff> aBuff, int aMaxStacks);
	Buff* GetBuff(BuffID anID);
	void ClearBuffs();

private:
	void	UpdateBuffs();

	void	UpdateInvincibility();
	void	FinishSwitchingInvulnerability(bool aInvulnerable);

	void	UpdateHealthRegen();
	void	UpdateBarrierDecay();
	void	UpdateShield();

	void	SpawnDamageNumbers(float aDamage, const CU::Vector3f* aPosition, eDamageType aDamageType);
	void	SpawnFloatingText(std::string aText, const CU::Vector3f* aPosition);

private:

	struct
	{
		float maxHP = 0;
		float baseMaxHp = 0;
		float hp = 0;
		float maxHpMod = 1.f;
		float damageModifier = 1.f;
		float bufferTime = 3.0f;
		float bufferTimer = 3.0f;
		float restoreSpeed = 2.0f;
		bool shouldRegenHealth = false;
	} myHealth;

	struct
	{
		float maxShield = 0.f;
		float shield = 0.f;
		float bufferTime = 3.0f;
		float bufferTimer = 3.0f;
		float restoreSpeed = 10.0f;
	} myShield;

	struct
	{
		float barrier = 0.f;
	} myBarrier;

	struct
	{
		float modifier = 0;
		float base = 0;
	} myArmor;

	struct
	{
		float time = 0;
	} myStun;

	struct
	{
		int timerIndex = -1;
		bool targetInvulnerability = false;
		bool inTransition = false;
		bool isInvulnerable = false;
	} myInvulnerability;

	struct DamageOverTime
	{
		DamageOverTime(float aDPS, float aDuration, float aTickLength) :
			DPS(aDPS), duration(aDuration), tickTimer(aTickLength), tickLength(aTickLength)
		{
		}

		float DPS = 0.f;
		float duration = 0.f;
		float tickTimer = tickLength;
		float tickLength = 0.333f;
	};

	struct Slow
	{
		Slow(float aSlowPercentage, float aSlowTime) :
			slowPercentage(aSlowPercentage), slowTime(aSlowTime)
		{
		}

		float slowPercentage = 0;
		float slowTime = 0;
		float slowTimer = slowTime;
	};

	std::vector<Slow> mySlow;
	std::vector<std::pair<BuffID, std::unique_ptr<Buff>>> myBuffs;

	std::function<void(float)>				myApplyHealingObserver;
	std::function<void(const Vec3f&)>		myApplyKnockObserver;
	std::function<void(float, float)>		myApplySlowObserver;
	std::function<void(float)>				myApplyStunObserver;
	std::vector<std::function<void(float)>>	myApplyDamageObserver;
	std::vector<std::function<void()>>		myDodgeObserver;
	std::vector<std::function<void(float)>>	myDeathObserver;
	std::function<void(float, float)>		myShieldChangeObserver;
	std::function<void(float)>				myBarrierChangeObserver;

	VFXRef myShieldVFXFRef;
	Engine::VFXComponent* myShieldVFX = nullptr;

	float myDodgeChance = 0.f;

	bool myShouldOneShotProtect = false;
	bool myIsInShield = false;
};

template <class T1, class T2>
_NODISCARD constexpr bool operator==(const std::pair<T1, T2>& aLeft, T1 aRight)
{
	return aLeft.first == aRight;
}