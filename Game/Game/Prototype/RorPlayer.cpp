#include "pch.h"
#include "RorPlayer.h"

#include "AI/PollingStation/PollingStationComponent.h"
#include "Components/HealthComponent.h"
#include "Engine/Shortcuts.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/GameObject/Components/CharacterControllerComponent.h"
#include "Engine/GameObject/Components/RigidBodyComponent.h"
#include "Engine/GameObject/Components/TextComponent.h"
#include "Engine/GameObject/Components/AudioComponent.h"
#include "Engine/Reflection/Reflector.h"
#include "Engine/Renderer/GraphicsEngine.h"

#include "../Engine/VFXParticleAttraction.h"

#include "Items/ItemManager.h"
#include "Camera/CameraController.h"
#include "Components/Interactable.h"
#include "Components/InteractableManager.h"

#include "HUD\HUDHandler.h"
#include "HUD\HUDAbility.h"
#include "HUD\HUDHealth.h"

RorPlayer::RorPlayer(GameObject* aGameObject)
	: Component(aGameObject)
{
}

RorPlayer::~RorPlayer()
{
	if (Main::Exists())
	{
		Main::SetPlayer(nullptr);
	}

}

void RorPlayer::Reflect(Engine::Reflector& aReflector)
{
	aReflector.Header("Controller");
	aReflector.Reflect(myStats.controller.height, "Controller Height");
	aReflector.Reflect(myStats.controller.height, "Controller Radius");
	aReflector.Reflect(myStats.controller.height, "Controller Radius");

	aReflector.Header("Mobility");
	aReflector.Reflect(myStats.mobility.movementSpeed, "Movement Speed");
	aReflector.Reflect(myStats.mobility.sprintMultiplier, "Sprint Speed Multiplier");
	aReflector.Reflect(myStats.mobility.jumpForce, "Jump Force");

	//aReflector.Header("Camera");
	//aReflector.Reflect(myStats.camera.mouseSensitivity, "Sensitivity");
	//aReflector.Reflect(myStats.camera.offset, "Camera Pivot Offset");
	//aReflector.Reflect(myStats.camera.cameraDistance, "Camera Distance");

	aReflector.Header("Pistol");
	aReflector.Reflect(myStats.pistol.damage, "Pistol Damage");
	aReflector.Reflect(myStats.pistol.rps, "Pistol RPS");
	aReflector.Reflect(myStats.pistol.bulletSpeed, "Pistol Speed");
	aReflector.Reflect(myStats.pistol.bulletRange, "Pistol Range");

	aReflector.Header("Piercing Shot");
	aReflector.Reflect(myStats.pierce.cooldown, "Pierce Cooldown");
	aReflector.Reflect(myStats.pierce.damage, "Pierce Damage");
	aReflector.Reflect(myStats.pistol.bulletSpeed, "Pierce Speed");
	aReflector.Reflect(myStats.pistol.bulletRange, "Pierce Range");

	aReflector.Header("AOE Shot");
	aReflector.Reflect(myStats.aoe.cooldown, "AOE Cooldown");
	aReflector.Reflect(myStats.aoe.bounces, "AOE Bounces");
	aReflector.Reflect(myStats.aoe.bounceRange, "AOE Bounce Range");
	aReflector.Reflect(myStats.aoe.damage, "AOE Damage");
	aReflector.Reflect(myStats.aoe.bulletSpeed, "AOE Speed");
	aReflector.Reflect(myStats.aoe.bulletRange, "AOE Range");

	aReflector.Header("Dash");
	aReflector.Reflect(myStats.dash.cooldown, "Dash Cooldown");
	aReflector.Reflect(myStats.dash.distance, "Dash Distance");
	aReflector.Reflect(myStats.dash.activeTime, "Dash Time");
}

void RorPlayer::Awake()
{
	auto* pollingStation = myGameObject->GetSingletonComponent<PollingStationComponent>();
	if (pollingStation)
		pollingStation->Setplayer(this);
	Main::SetPlayer(myGameObject);
}

void RorPlayer::Start()
{
	myGameObject->SetLayers(eLayer::PLAYER | eLayer::DAMAGEABLE, eLayer::GROUND | eLayer::DEFAULT, eLayer::ZONE);

	myController = myGameObject->AddComponent<Engine::CharacterControllerComponent>();
	myController->Init(myStats.controller.height, myStats.controller.radius);
	myController->SetGravity(982 * 2.f);
	myController->SetMovementSettings(0.05f);
	myController->ObserveTriggerEnter([](GameObject* obj)
		{
			LOG_INFO(LogType::Viktor) << "Player trigger enter";
		});

	myHealthComponent = myGameObject->AddComponent<HealthComponent>(myStats.health.health);
	myHealthComponent->ObserveDamage([&](float aDmg)
		{
			OnDamage(aDmg);
		});
	myHealthComponent->ObserveDeath([&](float aDmg)
		{
			OnDeath(aDmg);
		});
	myHealthComponent->ObserveKnockback([&](const Vec3f& aKnockback) {
			myController->Jump(aKnockback.y * 10.f);
		});

	myHealthComponent->ObserveShield([&](float aShield, float aMaxShield)
	{
		ShieldChange(aShield, aMaxShield);
	});
	myHealthComponent->ObserveBarrier([&](float aBarrier)
	{
		BarrierChange(aBarrier);
	});

	//myRb = myGameObject->AddComponent<Engine::RigidBodyComponent>();
	//myRb->Attach(Shape::Sphere(50));

	myHead = myGameObject->GetScene()->AddGameObject<GameObject>();
	myHead->SetName("Camera Pivot");
	myCamera = myGameObject->GetScene()->FindGameObject("MainCamera");
	//ConnectCamera();
	InitHead();

	//myDash = DashSkill(this);

	//myWeapon = PistolSkill(this);

	//mySecondary = PiercingSkill(this);
	//mySecondary.Setup(myStats.pierce.damage, myStats.pierce.bulletSpeed, myStats.pierce.bulletRange);

	//myUtility = AOESkill(this);

	myCameraController = myGameObject->GetComponent<CameraController>();
	assert(myCameraController && "Player has no camera controller :(");

	/* ----- HUD ----- */
	myHUDHandler = myGameObject->GetComponent<HUDHandler>();

	if (myHUDHandler)
	{
		//myHUDHandler->GetAbility(HUDHandler::AbilityType::PrimaryFire)->SetCooldownTime(myStats.pistol.rps);
		myHUDHandler->GetAbility(HUDHandler::AbilityType::SecondaryFire)->SetCooldownTime(mySecondary.GetCooldown());
		myHUDHandler->GetAbility(HUDHandler::AbilityType::Mobility)->SetCooldownTime(myDash.GetCooldown());
		myHUDHandler->GetAbility(HUDHandler::AbilityType::Utiliy)->SetCooldownTime(myUtility.GetCooldown());
	}
	myHUDHandler->GetHealth()->SetMaxHealth(myHealthComponent->GetMaxHealth());
	myHUDHandler->GetHealth()->SetHealth(myHealthComponent->GetHealth());

	// START
	// 
	//Add Audio Component
	myAudioComponent = myGameObject->AddComponent<Engine::AudioComponent>();

	// REGISTER SOUNDPATHS TO EVENTS
	myAudioComponent->AddEvent("Damage", /**/"event:/FOL/PLR/Footstep"/**/);
	myAudioComponent->AddEvent("Footstep", /**/"event:/FOL/PLR/Footstep"/**/);
	myAudioComponent->AddEvent("Sprint", /**/"event:/FOL/PLR/Footstep"/**/);
	myAudioComponent->AddEvent("Shoot", /**/"event:/FOL/PLR/Footstep"/**/);
	myAudioComponent->AddEvent("Jump", /**/"event:/FOL/PLR/Jump"/**/);
	myAudioComponent->AddEvent("Fall", /**/"event:/FOL/PLR/Footstep"/**/);
	myAudioComponent->AddEvent("Land", /**/"event:/FOL/PLR/Land"/**/);
	myAudioComponent->AddEvent("Dash", /**/"event:/FOL/PLR/Dash"/**/);
	myAudioComponent->AddEvent("Utility", /**/"event:/FOL/PLR/Footstep"/**/);
	myAudioComponent->AddEvent("Heal", /**/"event:/FOL/PLR/Footstep"/**/);
	myAudioComponent->AddEvent("Interact", /**/"event:/FOL/PLR/Footstep"/**/);
	myAudioComponent->AddEvent("Ability1Sound", /**/"event:/FOL/PLR/Footstep"/**/);
	myAudioComponent->AddEvent("Ability2Sound", /**/"event:/FOL/PLR/Footstep"/**/);
}

void RorPlayer::Execute(Engine::eEngineOrder aOrder)
{

	if (GetEngine().GetInputManager().IsKeyDown(C::KeyCode::K))
	{
		myHealthComponent->ApplyDamage(myHealthComponent->GetMaxHealth() * 0.1f);
		//Play Damage Sound
		myAudioComponent->PlayEvent("Damage");
	}

	if (GetEngine().GetInputManager().IsKeyDown(C::KeyCode::I))
	{
		myHealthComponent->AddBarrier(15.0f);
	}

	if (GetEngine().GetInputManager().IsKeyDown(C::KeyCode::O))
	{
		myHealthComponent->AddPermanentShield(15.0f);
	}
	//UpdateSkills();



	if (myCameraController->GetIfControllingCamera())
	{
		UpdateRotation();
		UpdateMovement();
		UpdateInteract();
	}
	UpdateCorsair();

	VFXParticleAttraction::SetAttractionPoint(GetTransform().GetPosition());
}

void RorPlayer::UpdateRotation()
{
	//auto deltaMouse = GetEngine().GetInputManager().GetMouseDeltaRaw();
	//static float xRot;
	//static float yRot;

	//yRot += deltaMouse.x * myStats.camera.mouseSensitivity;
	//xRot += deltaMouse.y * myStats.camera.mouseSensitivity;

	//CU::Matrix4f mrot = CU::Matrix4f::CreateRotationAroundX(xRot * Math::Deg2Rad) * CU::Matrix4f::CreateRotationAroundY(yRot * Math::Deg2Rad);

	//myGameObject->GetTransform().SetRotation(CU::Matrix4f::CreateRotationAroundY(yRot * Math::Deg2Rad));

	myHead->GetTransform().SetRotation(CU::Quaternion(myCameraController->GetCurrentRotation()));

	Vec3f rotation;
	rotation.y = myCameraController->GetCurrentRotation().y;

	myGameObject->GetTransform().SetRotation(rotation);

}

void RorPlayer::UpdateMovement()
{
	if (myIsMovementOverride)
		return;
	//myRb->SetVelocity({ 0,	myRb->GetVelocity().y,0 });

	if (GetEngine().GetInputManager().IsKeyDown(CU::KeyCode::LeftControl))
	{
		ToggleSprint(!myIsSprinting);
		//Play Sprint Sound
		myAudioComponent->PlayEvent("Sprint");
	}

	if (GetEngine().GetInputManager().IsKeyDown(CU::KeyCode::Space) && IsGrounded())
	{
		Jump();
		//Play Sprint Sound
		myAudioComponent->PlayEvent("Jump");
	}


	float speed = myStats.mobility.movementSpeed;
	if (myIsSprinting)
		speed *= myStats.mobility.sprintMultiplier;

	myCameraController->SetIsRunning(myIsSprinting);

	CU::Vector3f direction;

	if (GetEngine().GetInputManager().IsKeyPressed(CU::KeyCode::W))
	{
		direction += GetTransform().Forward();
	}
	if (GetEngine().GetInputManager().IsKeyPressed(CU::KeyCode::S))
	{
		direction -= GetTransform().Forward();
	}
	if (GetEngine().GetInputManager().IsKeyPressed(CU::KeyCode::A))
	{
		direction -= GetTransform().Right();
	}
	if (GetEngine().GetInputManager().IsKeyPressed(CU::KeyCode::D))
	{
		direction += GetTransform().Right();
	}
	GetTransform().Move(direction.GetNormalized() * speed * Time::DeltaTime);
}

void RorPlayer::UpdateCorsair()
{
	GDebugDrawer->DrawCircle3D(DebugDrawFlags::Always, myHead->GetTransform().GetPosition(), 1);


	// Fake with debug draws for now
	GDebugDrawer->DrawLine2D(DebugDrawFlags::Always, { 0.495f, 0.5f }, { 0.49f, 0.5f });
	GDebugDrawer->DrawLine2D(DebugDrawFlags::Always, { 0.505f, 0.5f }, { 0.51f, 0.5f });
	GDebugDrawer->DrawLine2D(DebugDrawFlags::Always, { 0.5f, 0.49f }, { 0.5f, 0.48f });
	GDebugDrawer->DrawLine2D(DebugDrawFlags::Always, { 0.5f, 0.51f }, { 0.5f, 0.52f });
}

void RorPlayer::UpdateInteract()
{
	if (Interactable* interactable = InteractableManager::Instance().SearchInteractable(GetTransform().GetPosition(), GetTransform().Forward()))
	{
		if (GetEngine().GetInputManager().IsKeyDown(interactable->GetKey()))
		{
			if (interactable->CanInteract())
			{
				interactable->Interact();
			}
		}
	}
}

void RorPlayer::UpdateSkills()
{
	myWeapon.Update();

	if (GetEngine().GetInputManager().IsMouseKeyPressed(Common::MouseButton::Left))
	{
		if (myWeapon.CanActivate())
		{
			//Play Primary Attack Sound
			myAudioComponent->PlayEvent("Shoot");
			myWeapon.Activate();
			//myHUDHandler->StartCooldown(HUDHandler::AbilityType::PrimaryFire);
		}
	}

	mySecondary.Update();

	if (GetEngine().GetInputManager().IsMouseKeyPressed(Common::MouseButton::Right))
	{
		if (mySecondary.CanActivate())
		{
			//Play Secondary Attack Sound
			myAudioComponent->PlayEvent("Shoot");
			mySecondary.Activate();
			myHUDHandler->StartCooldown(HUDHandler::AbilityType::SecondaryFire);
		}
	}

	myDash.Update();

	if (GetEngine().GetInputManager().IsKeyDown(Common::KeyCode::LeftShift))
	{
		if (myDash.CanActivate())
		{
			//Play Dash Sound
			myAudioComponent->PlayEvent("Dash");
			myDash.Activate();
			myHUDHandler->StartCooldown(HUDHandler::AbilityType::Mobility);
		}
	}

	myUtility.Update();

	if (GetEngine().GetInputManager().IsKeyDown(Common::KeyCode::R))
	{
		if (myUtility.CanActivate())
		{
			//Play Utility Sound
			myAudioComponent->PlayEvent("Utility");
			myUtility.Activate();
			myHUDHandler->StartCooldown(HUDHandler::AbilityType::Utiliy);
		}
	}
}

void RorPlayer::ShowCursor(bool aShow)
{
	GetEngine().SetCursorMode(aShow ? Engine::CursorMode::None : Engine::CursorMode::CenterLocked);
	GetEngine().SetCursorVisiblity(aShow);
}

void RorPlayer::InitHead()
{
	myHead->GetTransform().SetParent(&myGameObject->GetTransform());
	myHead->GetTransform().SetPositionLocal(myStats.camera.offset);
	myHead->GetTransform().SetRotationLocal(Quatf());
}

void RorPlayer::Jump()
{
	myController->Jump(myStats.mobility.jumpForce);
	//const auto velocity = myRb->GetVelocity();
	//myRb->SetVelocity({ velocity.x, myStats.mobility.jumpForce, velocity.z });
}

void RorPlayer::ToggleSprint(bool aValue)
{
	myIsSprinting = aValue;
}

bool RorPlayer::IsFalling() const
{
	return false; // myRb->GetVelocity().y < 0;
}

bool RorPlayer::IsGrounded() const
{
	return myController->IsGrounded();
}

CameraController* RorPlayer::GetCameraController()
{
	return myCameraController;
}

void RorPlayer::ToggleOverrideMovement(bool aValue)
{
	myIsMovementOverride = aValue;
}

void RorPlayer::StatAddDamageMod(float aModifier)
{
	myStats.damageModifier += aModifier;

	//myWeapon.SetDmg(myStats.pistol.damage * myStats.damageModifier);
	//mySecondary.SetDamage(myStats.pierce.damage * myStats.damageModifier);
	myUtility.SetDamage(myStats.aoe.damage * myStats.damageModifier);
}

void RorPlayer::StatAddJumpForce(float aForce)
{
	myStats.mobility.jumpForce += aForce;
}

void RorPlayer::StatAddPistolRPS(float aRPS)
{
	myStats.pistol.rps += aRPS;
	//myWeapon.SetRPS(myStats.pistol.rps);
}

void RorPlayer::StatAddAOEBounce(int someBounces)
{
	myStats.aoe.bounces += someBounces;
	myUtility.SetBounces(myStats.aoe.bounces);
}

void RorPlayer::StatAddDashDistance(float someDistance)
{
	myStats.dash.distance += someDistance;
	myDash.SetDistance(myStats.dash.distance);
}

void RorPlayer::CheckGrounded()
{
}

void RorPlayer::OnDamage(float aDmg)
{
	ItemEventData data;
	data.damage = aDmg;
	Main::GetItemManager().PostEvent(eItemEvent::OnPlayerDamaged, &data);

	//Play Damage Sound
	myAudioComponent->PlayEvent("Damage");

	LOG_INFO(LogType::Game) << "Player damaged: " << aDmg;

	myHUDHandler->GetHealth()->SetHealth(myHealthComponent->GetHealth());
}

void RorPlayer::OnDeath(float aDmg)
{
	LOG_INFO(LogType::Game) << "Player Died";
	//DisconnectCamera();
	//myGameObject->Destroy();

	GetTransform().SetPosition({ 0,1000,0 });

	myController->SetPosition({ 0,1000,0 });
	myHealthComponent->ResetHealthToMax();

	//myHUDHandler->SetHealthPercent(0.0f); //TODO: Because we just reset the player on death, set HUD to max health, when we change in the future, set HUD to 0hp
	myHUDHandler->GetHealth()->SetHealth(myHealthComponent->GetHealth());
}

void RorPlayer::ShieldChange(float aShield, float aMaxShield)
{
	myHUDHandler->GetHealth()->SetShields(aShield, aMaxShield);
}

void RorPlayer::BarrierChange(float aBarrier)
{
	myHUDHandler->GetHealth()->SetBarrier(aBarrier);
}
