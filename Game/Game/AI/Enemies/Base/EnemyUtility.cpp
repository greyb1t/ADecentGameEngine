#include "pch.h"
#include "EnemyUtility.h"
#include "Engine/Animation/AnimationController.h"

void Enemy::EnemyTrigger(Engine::AnimationController& animController, const std::string& aTriggerName)
{
	animController.Trigger(aTriggerName);
}

void Enemy::SetEnemyCondition(Engine::AnimationController& animController, const std::string& aConditionName,
	bool aValue)
{
	animController.SetBool(aConditionName, aValue);
}
