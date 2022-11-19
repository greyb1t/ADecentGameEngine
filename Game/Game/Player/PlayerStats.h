#pragma once
#include "UI/PauseMenu/StatsDisplay.h"

struct PlayerStats
{
	enum eVariable
	{
		HEALTH,
		MOVEMENT_SPEED,
		SPRINT_MULTIPLIER, // *
		JUMP_FORCE,
		JUMP_AMOUNT, // *
		DAMAGE,
		CRIT_CHANCE, // *
		CRIT_DAMAGE_MULTIPLIER, // *
		ATTACK_SPEED,
		COOLDOWN_REDUCTION, // *
		GRAVITY,
		TERMINAL_VELOCITY,
		STEADINESS_RECOVERY_RATE, // *
		VELOCITY_DISSIPATE_MIN, // *
		FALL_DAMAGE_MIN, // *
		FALL_DAMAGE_MAX, // *
		FALL_SPEED_MIN, // *
		FALL_SPEED_MAX, // *
		FALL_DAMAGE_ACTIVE,
	};

public:
	void ObserveChange(const std::function<void(eVariable)>& aObserver);

	float	GetHealth() const ;
	void	SetHealth(float aValue);
	float&	HealthRef();

	float	GetMovementSpeed() const ;
	void	SetMovementSpeed(float aValue);
	float&	MovementSpeedRef();

	float	GetSprintMultiplier() const ;
	void	SetSprintMultiplier(float aValue);
	float&	SprintMultiplierRef();

	float	GetJumpForce() const ;
	void	SetJumpForce(float aValue);
	float&	JumpForceRef();

	float	GetJumpAmount() const ;
	void	SetJumpAmount(float aValue);
	void	AddJumpAmount(float aValue);
	float&	JumpAmountRef();

	float	GetAmountJumped() const;
	void	SetAmountJumped(float aValue);
	float& AmountJumpedRef();

	float	GetDamage() const ;
	float&	DamageRef();

	void	SetBaseDamage(float aValue);
	float	GetBaseDamage();
	void	SetDamageMod(float aValue);
	float	GetDamageMod();

	float	GetCritChance() const ;
	void	SetCritChance(float aValue);
	float&	CritChance();
	
	float	GetDodgeChance() const ;
	void	SetDodgeChance(float aValue);
	void	AddDodgeChance(float aValue);
	float&	DodgeChanceRef();

	float	GetCritDamageMultiplier() const ;
	void	SetCritDamageMultiplier(float aValue);
	float&	CritDamageMultiplierRef();

	float	GetAttackSpeed() const ;
	void	SetAttackSpeed(float aValue);
	float&	AttackSpeedRef();

	float	GetCooldownMultiplier() const ;
	void	SetCooldownMultiplier(float aValue);
	void	AddCooldownMultiplier(float aValue);
	float&	CooldownMultiplierRef();

	float	GetGravity() const ;
	void	SetGravity(float aValue);
	float&	GravityRef();

	float	GetTerminalVelocity() const ;
	void	SetTerminalVelocity(float aValue);
	float&	TerminalVelocityRef();

	float	GetSteadinessRecoveryRate() const ;
	void	SetSteadinessRecoveryRate(float aValue);
	float&  SteadinessRecoveryRateRef();

	float	GetVelocityDissipateMin() const;
	void	SetVelocityDissipateMin(float aValue);
	float&  VelocityDissipateMinRef();

	float	GetMovementLag() const;
	void	SetMovementLag(float aValue);
	float&	GetMovementLagRef();

	float	GetFallDamageMin() const;
	void	SetFallDamageMin(float aValue);
	float&	FallDamageMinRef();

	float	GetFallDamageMax() const;
	void	SetFallDamageMax(float aValue);
	float&	FallDamageMaxRef();

	float	GetFallSpeedMin() const;
	void	SetFallSpeedMin(float aValue);
	float&	FallSpeedMinRef();

	float	GetFallSpeedMax() const;
	void	SetFallSpeedMax(float aValue);
	float&	FallSpeedMaxRef();

	bool	GetIsFallDamageActive() const;
	void	SetIsFallDamageActive(bool aValue);
	bool&	IsFallDamageActiveRef();

	uint32_t GetRunAnimationSpeedUpCount() const;
	void IncrementRunAnimationSpeedUpCount();

private:
	void Change(eVariable aVariable);

	std::function<void(eVariable)> myObserver;

	float health = 100;

	float movementSpeed = 700;
	float sprintMultiplier = 2.f;
	float movementLag = 12.f;
	float jumpForce = 400;
	float jumpAmount = 3;
	float amountJumped = 0;

	float dodgeChance = 0.f;

	float damage = 10.f;
	float baseDamage = 10.f;
	float damageModifier = 1.f;
	float critChance = 0.05f;
	float critDamageMultiplier = 2.f;
	float attackSpeed = 1;

	float cooldownMultiplier = 0.0f;

	float gravity = 982;
	float terminalVelocity = 982 * 3;
	float fallDamageMin = 0;
	float fallDamageMax = 80;
	float fallSpeedMin = 1000;
	float fallSpeedMax = 15000;
	bool fallDamageActive = true;

	float steadinessRecoveryRate = 1;
	float velocityDissipateMin = 100;
	float velocityDissipatePercentage = 0.3f;

	uint32_t myRunAnimationSpeedUpCount = 0;
};

inline void PlayerStats::ObserveChange(const std::function<void(eVariable)>& aObserver)
{
	myObserver = aObserver;
}

inline float PlayerStats::GetHealth() const
{
	return health;
}

inline void PlayerStats::SetHealth(float aValue)
{
	Change(HEALTH);
	health = aValue;
}

inline float& PlayerStats::HealthRef()
{
	return health;
}

inline float PlayerStats::GetMovementSpeed() const
{
	return movementSpeed;
}

inline void PlayerStats::SetMovementSpeed(float aValue)
{
	Change(MOVEMENT_SPEED);
	movementSpeed = aValue;
}

inline float& PlayerStats::MovementSpeedRef()
{
	return movementSpeed;
}

inline float PlayerStats::GetSprintMultiplier() const
{
	return sprintMultiplier;
}

inline void PlayerStats::SetSprintMultiplier(float aValue)
{
	Change(SPRINT_MULTIPLIER);
	sprintMultiplier = aValue;
}

inline float& PlayerStats::SprintMultiplierRef()
{
	return sprintMultiplier;
}

inline float PlayerStats::GetJumpForce() const
{
	return jumpForce;
}

inline void PlayerStats::SetJumpForce(float aValue)
{
	Change(JUMP_FORCE);
	jumpForce = aValue;
}

inline float& PlayerStats::JumpForceRef()
{
	return jumpForce;
}

inline float PlayerStats::GetJumpAmount() const
{
	return jumpAmount;
}

inline void PlayerStats::SetJumpAmount(float aValue)
{
	Change(JUMP_AMOUNT);
	jumpAmount = aValue;
}
inline void PlayerStats::AddJumpAmount(float aValue)
{
	Change(JUMP_AMOUNT);
	jumpAmount += aValue;
}

inline float& PlayerStats::JumpAmountRef()
{
	return jumpAmount;
}

inline float PlayerStats::GetAmountJumped() const
{
	return amountJumped;
}

inline void PlayerStats::SetAmountJumped(float aValue)
{
	amountJumped = aValue;
}

inline float& PlayerStats::AmountJumpedRef()
{
	return amountJumped;
}

inline float PlayerStats::GetDamage() const
{
	return damage;
}

inline void PlayerStats::SetBaseDamage(float aValue)
{
	Change(DAMAGE);
	baseDamage = aValue;
	damage = baseDamage * damageModifier;
}

inline float PlayerStats::GetBaseDamage()
{
	return baseDamage;
}

inline void PlayerStats::SetDamageMod(float aValue)
{
	damageModifier = aValue;
	damage = baseDamage * damageModifier;
}

inline float PlayerStats::GetDamageMod()
{
	return damageModifier;
}

inline float& PlayerStats::DamageRef()
{
	return damage;
}

inline float PlayerStats::GetCritChance() const
{
	return critChance;
}

inline void PlayerStats::SetCritChance(float aValue)
{
	Change(CRIT_CHANCE);
	critChance = aValue;
}

inline float& PlayerStats::CritChance()
{
	return critChance;
}

inline float PlayerStats::GetDodgeChance() const
{
	return dodgeChance;
}

inline void PlayerStats::SetDodgeChance(float aValue)
{
	dodgeChance = aValue;
}

inline void PlayerStats::AddDodgeChance(float aValue)
{
	dodgeChance += aValue;
}

inline float& PlayerStats::DodgeChanceRef()
{
	return dodgeChance;
}

inline float PlayerStats::GetCritDamageMultiplier() const
{
	return critDamageMultiplier;
}

inline void PlayerStats::SetCritDamageMultiplier(float aValue)
{
	Change(CRIT_DAMAGE_MULTIPLIER);
	critDamageMultiplier = aValue;
}

inline float& PlayerStats::CritDamageMultiplierRef()
{
	return critDamageMultiplier;
}

inline float PlayerStats::GetAttackSpeed() const
{
	return attackSpeed;
}

inline void PlayerStats::SetAttackSpeed(float aValue)
{
	Change(ATTACK_SPEED);
	attackSpeed = aValue;
}

inline float& PlayerStats::AttackSpeedRef()
{
	return attackSpeed;
}

inline float PlayerStats::GetCooldownMultiplier() const
{
	return cooldownMultiplier;
}

inline void PlayerStats::SetCooldownMultiplier(float aValue)
{
	cooldownMultiplier = aValue;
	cooldownMultiplier = CU::Clamp(0.05f, 1.f, cooldownMultiplier);
	Change(COOLDOWN_REDUCTION);
}

inline void PlayerStats::AddCooldownMultiplier(float aValue)
{
	cooldownMultiplier += aValue;
	cooldownMultiplier = CU::Clamp(0.05f, 1.f, cooldownMultiplier);
	Change(COOLDOWN_REDUCTION);
}

inline float& PlayerStats::CooldownMultiplierRef()
{
	return cooldownMultiplier;
}

inline float PlayerStats::GetGravity() const
{
	return gravity;
}

inline void PlayerStats::SetGravity(float aValue)
{
	Change(GRAVITY);
	gravity = aValue;
}

inline float& PlayerStats::GravityRef()
{
	return gravity;
}

inline float PlayerStats::GetTerminalVelocity() const
{
	return terminalVelocity;
}

inline void PlayerStats::SetTerminalVelocity(float aValue)
{
	Change(TERMINAL_VELOCITY);
	terminalVelocity = aValue;
}

inline float& PlayerStats::TerminalVelocityRef()
{
	return terminalVelocity;
}

inline float PlayerStats::GetSteadinessRecoveryRate() const
{
	return steadinessRecoveryRate;
}

inline void PlayerStats::SetSteadinessRecoveryRate(float aValue)
{
	Change(STEADINESS_RECOVERY_RATE);
	steadinessRecoveryRate = aValue;
}

inline float& PlayerStats::SteadinessRecoveryRateRef()
{
	return steadinessRecoveryRate;
}

inline float PlayerStats::GetVelocityDissipateMin() const
{
	return velocityDissipateMin;
}

inline void PlayerStats::SetVelocityDissipateMin(float aValue)
{
	Change(VELOCITY_DISSIPATE_MIN);
	velocityDissipateMin = aValue;
}

inline float& PlayerStats::VelocityDissipateMinRef()
{
	return velocityDissipateMin;
}

inline void PlayerStats::Change(eVariable aVariable)
{
	if (myObserver)
		myObserver(aVariable);
}

inline float PlayerStats::GetMovementLag() const
{
	return movementLag;
}

inline void PlayerStats::SetMovementLag(float aValue)
{
	movementLag = aValue;
}

inline float& PlayerStats::GetMovementLagRef()
{
	return movementLag;
}

inline float PlayerStats::GetFallDamageMin() const
{
	return fallDamageMin;
}

inline void PlayerStats::SetFallDamageMin(float aValue)
{
	fallDamageMin = aValue;
}

inline float& PlayerStats::FallDamageMinRef()
{
	return fallDamageMin;
}

inline float PlayerStats::GetFallDamageMax() const
{
	return fallDamageMax;
}

inline void PlayerStats::SetFallDamageMax(float aValue)
{
	fallDamageMax = aValue;
}

inline float& PlayerStats::FallDamageMaxRef()
{
	return fallDamageMax;
}

inline float PlayerStats::GetFallSpeedMin() const
{
	return fallSpeedMin;
}

inline void PlayerStats::SetFallSpeedMin(float aValue)
{
	fallSpeedMin = aValue;
}

inline float& PlayerStats::FallSpeedMinRef()
{
	return fallSpeedMin;
}

inline float PlayerStats::GetFallSpeedMax() const
{
	return fallSpeedMax;
}

inline void PlayerStats::SetFallSpeedMax(float aValue)
{
	fallSpeedMax = aValue;
}

inline float& PlayerStats::FallSpeedMaxRef()
{
	return fallSpeedMax;
}

inline bool PlayerStats::GetIsFallDamageActive() const
{
	return fallDamageActive;
}

inline void PlayerStats::SetIsFallDamageActive(bool aValue)
{
	fallDamageActive = aValue;
}

inline bool& PlayerStats::IsFallDamageActiveRef()
{
	return fallDamageActive;
}

inline uint32_t PlayerStats::GetRunAnimationSpeedUpCount() const
{
	return myRunAnimationSpeedUpCount;
}

inline void PlayerStats::IncrementRunAnimationSpeedUpCount()
{
	myRunAnimationSpeedUpCount++;
}