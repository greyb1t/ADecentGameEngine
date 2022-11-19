#pragma once
#include "Engine\GameObject\Components\Component.h"
#include "Skills/AOESkill.h"
#include "Skills/DashSkill.h"
#include "Skills/PiercingSkill.h"
#include "Skills/PistolSkill.h"

class Interactable;

namespace Engine
{
	class TextComponent;
	class SpriteComponent;
	class CharacterControllerComponent;
	class RigidBodyComponent;
	class AudioComponent;
}

class CameraController;
class HealthComponent;
class HUDHandler;

struct RorPlayerStats
{
	struct
	{
		float height = 130.f;
		float radius = 25.f;
	} controller;

	struct
	{
		float movementSpeed = 900;
		float sprintMultiplier = 1.33f;

		float jumpForce = 300;
	} mobility;

	struct
	{
		float mouseSensitivity = .6f;
		Vec3f offset = { 0, 100, 0 };
		float cameraDistance = 700;
	} camera;

    struct
    {
        float damage = 10;
        float rps = 3;
        float bulletSpeed = 1000;
        float bulletRange = 5000;
    } pistol;

    struct
    {
        int bounces = 6;
        float bounceRange = 800;
        float cooldown = 7;
        float damage = 10;
        float bulletSpeed = 1000;
        float bulletRange = 3000;
    } aoe;

    struct
    {
		float cooldown = 3;
        float damage = 50;
        float bulletSpeed = 10000;
        float bulletRange = 5000;
    } pierce;

	struct
	{
		float distance = 500;
		float activeTime = .1f;
		float cooldown = 3.f;
	} dash;

	struct
	{
		float health = 100;
	} health;

	float damageModifier = 1.f;
};

class RorPlayer :
	public Component
{
public:
	COMPONENT(RorPlayer, "RoR Player");

	RorPlayer() = default;
	RorPlayer(GameObject*);
	~RorPlayer();

	void Reflect(Engine::Reflector& aReflector) override;

	void Awake() override;
    void Start() override;
    void Execute(Engine::eEngineOrder aOrder) override;

    void UpdateRotation();
    void UpdateMovement();
    void UpdateCorsair();
    void UpdateInteract();

	void UpdateSkills();

	void ShowCursor(bool);

	void InitHead();

	void Jump();
	void ToggleSprint(bool);

	bool IsFalling() const;
	bool IsGrounded() const;

	GameObject* GetCamera() const { return myCamera; }
	GameObject* GetHead() const { return myHead; }
	Engine::CharacterControllerComponent* GetController() const { return myController; }
	//Engine::RigidBodyComponent* GetRigidBody() const { return myRb; }

	CameraController* GetCameraController();

	void ToggleOverrideMovement(bool);
	bool IsMovementOverride() const { return myIsMovementOverride; }

	void StatAddDamageMod(float aModifier);
	void StatAddJumpForce(float aForce);
	void StatAddPistolRPS(float aRPS);
	void StatAddAOEBounce(int someBounces);
	void StatAddDashDistance(float someDistance);

	// Maybe add so we can subscribe on things like interruption?
	// So we tell skills to interrupt and so on
private:
	void CheckGrounded();

	void OnDamage(float aDmg);
	void OnDeath(float aDmg);

	void ShieldChange(float aShield, float aMaxShield);
	void BarrierChange(float aBarrier);

private:
	RorPlayerStats myStats;
	//Engine::RigidBodyComponent* myRb = nullptr;
	Engine::CharacterControllerComponent* myController = nullptr;
	GameObject* myHead = nullptr;
	GameObject* myCamera = nullptr;

	bool myIsSprinting = false;

	PistolSkill myWeapon;
	PiercingSkill mySecondary;
	DashSkill myDash;
	AOESkill myUtility;

	bool myIsMovementOverride = false;

	CameraController* myCameraController = nullptr;

	struct
	{
		Engine::SpriteComponent* healthBar = nullptr;
	} myHud;

	HUDHandler* myHUDHandler = nullptr;
	HealthComponent* myHealthComponent = nullptr;
	Engine::AudioComponent* myAudioComponent = nullptr;
};