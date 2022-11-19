#pragma once
#include "Engine/GameObject/Components/Component.h"
#include "Engine\ResourceManagement\ResourceRef.h"

namespace Engine
{
	class CharacterControllerComponent;
}

enum class eAIType;
class BT_Blackboard;
class HealthComponent;

class EnemyHealth : public Component
{
public:
	EnemyHealth();
	EnemyHealth(GameObject* aGameObject, float aMaxHealth);
	~EnemyHealth() override;

	void Init(BT_Blackboard* aBlackboard, std::function<void()> aOnDeathFunction = nullptr);

	Component* Clone() const override { return DBG_NEW EnemyHealth(*this); }
	void Reflect(Engine::Reflector& aReflector) override;

	void Start() override;

	void Execute(Engine::eEngineOrder aOrder) override;

	void OnDamage(float aDamage);
	void OnDeath(float aDamage);

	void SetMaxHealth(const float aHealth);

	//void CallDeathFunction();
	void Kill();

	void SetEyeGameObject(GameObject* aGameobject);
	void SetEyeEffectGameObject(GameObject* aGameobject);

	HealthComponent* GetHealthComponent() const;
private:
	HealthComponent* myHealthComponent = nullptr;
	BT_Blackboard* myBlackboard = nullptr;
	float myMaxHealth = 0.f;
	std::function<void()> myDeathFunction;

	float myDeathTimer = 0.f;
	float myDeathDuration = 10.f;
	float myOnDamageTimer = 0.f;
	float myOnDamageDuration = 0.1f;
	bool myActivateOnDamgeTimer = false;
	GameObject* myEyeObject = nullptr;
	GameObject* myEyeEffectObject = nullptr;

	float myDeathSinkTimer = 0.f;
	float myDeathSinkTime = 2.f;
	Engine::VFXComponent* myDecayVFX = nullptr;

};