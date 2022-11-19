#pragma once

struct PlayerSettings 
{
	struct {
		float airStrength = 0.1f;
		float groundStrength = 0.8f;
		float stopThreshold = 100;
	} knockback;

	struct {
		float invisibleDelay = 0.1f;
	} death;

	struct {
		float timeToInvisible = 0.5f;
		float timeToVisible = 0.1f;

		struct {
			float rotationAmount = 17.5f;
			AnimationCurveRef rotationLerp;
			float recoilStrength = 30.f;
			AnimationCurveRef recoilLerp;
		} primaryRotation;
	} magicCircle;

	struct {
		Vec3f offset = {30,0,0};
	} leftHand;
	struct {
		Vec3f offset = {30,0,0};
	} rightHand;

	struct {
		float upperBodyWeightSpeed = 3;
		float movementStartSpeed = 700.f;
	} animation;

	struct {
		float combatReadyTime = 1;
	} combat;

	struct {
		struct {
			float timeActive = 0.07f;
			AnimationCurveRef sizeCurve;
			AnimationCurveRef alphaCurve;
		} hitmark;

		struct {
			AnimationCurveRef rotationSpeedCurve;
			AnimationCurveRef returnLerp;
			float returnTime = .2f;
		} secondary;
	} crossHair;
};