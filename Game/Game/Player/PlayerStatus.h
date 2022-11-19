#pragma once

struct PlayerStatus
{
	Vec3f movementDirection = { 0, 0, 0 };
	bool isAlive = true;
	bool isGrounded = false;
	bool isMoving = false; 
	bool isFalling = false;
	bool isSprinting = false;
	bool isJumpActive = true;
	bool overrideMovement = false;
	bool overrideVelocity = false;
	bool overrideHUD = false;
	bool isAiming = false;
	float upperBodyWeightGoal = 0;
	float upperBodyWeight = 0;
};