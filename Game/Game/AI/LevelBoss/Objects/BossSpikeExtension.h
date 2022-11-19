#pragma once

#include "Engine/GameObject/Components/Component.h"

namespace Engine
{
	class Collider;
	class AnimatorComponent;
}

class BossSpikeExtension : public Component
{
	COMPONENT(BossSpikeExtension, "Boss Spike Extension");
public:
	BossSpikeExtension() = default;
	~BossSpikeExtension() = default;

	void Awake() override;
	void Start() override;

	void Reflect(Engine::Reflector& aReflector) override;

	void SetDamage(const float aMin, const float aMax);
	void SetForce(const float aForce);

private:
	Engine::GameObjectRef myGameObjectWithCollider;
	Engine::Collider* myCollider = nullptr;
	Engine::AudioComponent* myAudioComponent = nullptr;
	Engine::AnimatorComponent* myAnimator = nullptr;

	float myMinDamage = 10.0f;
	float myMaxDamage = 30.0f;
	float myForce = 10.0f;
	bool myHasHitPlayer = false;
};

