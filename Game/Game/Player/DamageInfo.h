#pragma once

struct DamageInfo
{
	DamageInfo() = default;
	DamageInfo(float aDamage, float aCritChance, float aCritMultiplier);

	inline float GetDamage();
	inline bool IsCrit() const;

	float damage = 1;
	float critChance = 0;
	float critMultiplier = 0;
	bool isCrit = false;
};

inline DamageInfo::DamageInfo(float aDamage, float aCritChance, float aCritMultiplier)
	: damage(aDamage), critChance(aCritChance), critMultiplier(aCritMultiplier)
{}

inline float DamageInfo::GetDamage()
{
	isCrit = (Random::RandomFloat(0.f, 1.f) < critChance);
	return damage * (isCrit ? critMultiplier : 1.f);
}

inline bool DamageInfo::IsCrit() const
{
	return isCrit;
}
