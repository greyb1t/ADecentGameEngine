#pragma once
#include "Skill.h"
#include "Player/SkillStats.h"

class DashSkill
	: public Skill
{
public:
	DashSkill() = default;
	DashSkill(Player*);
	void Activate() override;
	void Update() override;

	void Observe(const std::function<void()>& aObserver);

	inline void SetDistance(float aDistance) { myDashDistance = aDistance; }
private:
	void StartDash();
	void DashUpdate();
	void EndDash();

private:
	float	myDashDistance = 600;
	float	myDashTime = .1f;

	bool	myIsDashing = false;
	float	myDistanceDashed = 0;
	Vec3f	myDashDirection;
	std::function<void()> myOnActivate;
};

