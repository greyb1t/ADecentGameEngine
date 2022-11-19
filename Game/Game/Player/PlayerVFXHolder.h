
#pragma once

struct PlayerVFXHolder 
{
	struct {
		GameObjectPrefabRef mainRune;
		GameObjectPrefabRef smallRune;
	} magicCircle;

	VFXRef heal;
	VFXRef airJump;
	VFXRef death;

	/* DASH */
	VFXRef dashOutVFX;
	VFXRef dashVFX;

	/* PRIMARY */
	struct {
		VFXRef fire;
		VFXRef hit;
		GameObjectPrefabRef projectile;
	} primary;

	/* SECONDARY */
	struct {
		GameObjectPrefabRef projectilePrefab;
		VFXRef projectile;
		VFXRef chargeUp;
		VFXRef muzzleFlash;
		VFXRef hit;
		GameObjectPrefabRef explosionPrefab;
		AnimationCurveRef explosionScaling;
		VFXRef explosion;
	} secondary;

	/* SECONDARY */
	struct {
		VFXRef projectile;
		VFXRef hit;
	} utility;

	struct {
		Vec3f dustOffset = { 0, 0, 0 };
		VFXRef normalDust;
	} sprintDust;
};