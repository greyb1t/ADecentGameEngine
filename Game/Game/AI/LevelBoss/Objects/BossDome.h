#pragma once

#include "Engine/GameObject/Components/Component.h"

class LevelBoss;
class RayCastHit;

class BossDome : public Component
{
	COMPONENT(BossDome, "Boss Dome");
public:
	BossDome() = default;
	~BossDome() = default;

	void Awake() override;

	bool Init(LevelBoss* aLevelBoss);
	void Execute(Engine::eEngineOrder aOrder) override;

	void OnBeginFight();
	void OnDeath();

	void OnHit(RayCastHit& aHit, GameObject* aCollidedWith);

private:
	bool InitComponents();
	bool InitStartValues();

	void OnUpdateTransitions();
	void OnCheckWithinBounds();

	void OnToggleCollision(const bool aIsWithinBounds);
	void OnToggleEngagement(const bool aIsWithinBounds);

private:
	LevelBoss* myBoss = nullptr;
	Engine::ModelComponent* myDomeModel = nullptr;
	Engine::Collider* myCollider = nullptr;

	enum class State
	{
		Appear,
		Disappear
	} myState = State::Appear;

	float myDomeProgress = 0.0f;
	float myDomeDuration = 2.0f;
	float myRadius = 0.0f;

	bool myWasEngaged = false;
};

