#pragma once
#include "ShootState.h"
#include "Player/State.h"
#include "Player/RotationHandler.h"

namespace Engine
{
	class VFXComponent;
}

class MovementState :
    public ShootState
{
public:
	MovementState(Player* aPlayer);

	void Enter() override;
	void Exit() override;	
	void Update() override;

	void UpdateMovement();
	void UpdateRotation();

	void Jump();
	void ToggleSprint();
	void Sprint();
	void CancelSprint();

	[[nodiscard]] float GetMoveSpeed() const;

private:
	RotationHandler myRotator;
	Vec3f myMovementVelocity = { 0, 0, 0 };
	Transform* myModelTransform;

	Engine::VFXComponent* mySprintVFX = nullptr;
};

