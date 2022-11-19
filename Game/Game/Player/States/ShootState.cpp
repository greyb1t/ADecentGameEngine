#include "pch.h"
#include "ShootState.h"

#include "Engine/GameObject/Components/AnimatorComponent.h"
#include "Player/Player.h"

ShootState::ShootState(Player* aPlayer)
	: State(aPlayer)
{

}

void ShootState::Enter()
{
}

void ShootState::Exit()
{
}

void ShootState::Update()
{
	/*switch (myCombatState)
	{
	case eCombatState::COMBAT_IDLE: LOG_INFO(LogType::Viktor) << "IDLE"; break;
	case eCombatState::COMBAT_READY: LOG_INFO(LogType::Viktor) << "READY"; break;
	case eCombatState::PRIMARY_SHOOTING: LOG_INFO(LogType::Viktor) << "SHOOTING"; break;
	case eCombatState::SECONDARY_CHARGING: LOG_INFO(LogType::Viktor) << "CHARGING"; break;
	default: break;
	}*/

	bool inCombat = myCombatState != eCombatState::COMBAT_IDLE;
	
	GetPlayer()->GetAnimatorComponent().GetController().SetBool("InCombat", inCombat);
	GetPlayer()->GetAnimatorComponent().GetController().SetBool("IsShooting", myCombatState == eCombatState::PRIMARY_SHOOTING);

	UpdateCombatTimer();
	GetPlayer()->GetStatus().isAiming = inCombat && !GetPlayer()->GetStatus().isSprinting;

	if (myCombatState == eCombatState::SECONDARY_CHARGING)
	{
		UpdateSecondary();
		UpdateMobility();
		return;
	}

	UpdateMobility();
	UpdateSecondary();
	UpdateUtility();
	UpdatePrimary();
}

ShootState::eCombatState ShootState::GetState()
{
	return myCombatState;
}

void ShootState::ResetCombatTimer()
{
	myCombatReadyTimer = GetSettings().combat.combatReadyTime;
}

void ShootState::UpdateCombatTimer()
{
	if (myCombatState != eCombatState::COMBAT_IDLE)
	{
		myCombatReadyTimer -= Time::DeltaTime;
		if (myCombatReadyTimer <= 0)
		{
			myCombatState = eCombatState::COMBAT_IDLE;
		}
	}
}

void ShootState::UpdatePrimary()
{
	if (GetEngine().GetInputManager().IsMouseKeyPressed(Common::MouseButton::Left))
	{
		auto& skill = GetPlayer()->GetPrimarySkill();
		if (skill.CanActivate())
		{
			if (myCombatState == eCombatState::SECONDARY_CHARGING)
				return;

			myCombatState = eCombatState::PRIMARY_SHOOTING;
			ResetCombatTimer();
			skill.Activate();
			GetStatus().isSprinting = false;
		}
	} else if (myCombatState == eCombatState::PRIMARY_SHOOTING)
	{
		myCombatState = eCombatState::COMBAT_READY;
	}
}

void ShootState::UpdateSecondary()
{
	if (myCombatState == eCombatState::SECONDARY_CHARGING)
	{
		ResetCombatTimer();
		GetStatus().isSprinting = false;
		if (!GetEngine().GetInputManager().IsMouseKeyPressed(Common::MouseButton::Right))
		{
			ShootSecondary();
		}
		return;
	}

	if (GetEngine().GetInputManager().IsMouseKeyDown(Common::MouseButton::Right))
	{
		ActivateSecondary();
	}
}

void ShootState::UpdateMobility()
{
	if (GetEngine().GetInputManager().IsKey(Common::KeyCode::LeftShift))
	{
		auto& skill = GetPlayer()->GetMobilitySkill();
		if (skill.CanActivate())
		{
			if (myCombatState == eCombatState::SECONDARY_CHARGING)
			{
				InterruptSecondary();
			}

			skill.Activate();
		}
	}
}

void ShootState::UpdateUtility()
{
	if (GetEngine().GetInputManager().IsKey(Common::KeyCode::R))
	{
		auto& skill = GetPlayer()->GetUtilitySkill();
		if (skill.CanActivate())
		{
			skill.Activate();
			GetPlayer()->GetAnimatorComponent().GetController().Trigger("UtilityActivate");
			GetStatus().upperBodyWeight = 1;
			GetStatus().upperBodyWeightGoal = 1;
			myCombatState = eCombatState::COMBAT_READY;
			ResetCombatTimer();
		}
	}
}


void ShootState::ActivateSecondary()
{
	auto& skill = GetPlayer()->GetSecondarySkill();
	if (!skill.CanActivate())
		return;

	ResetCombatTimer();
	myCombatState = eCombatState::SECONDARY_CHARGING;
	GetPlayer()->GetAnimatorComponent().GetController().SetBool("IsSecondaryCharging", true);
	GetPlayer()->GetCrosshair().EnterCharge();
	skill.Activate();
	GetPlayer()->GetLeftArm().ActivateSecondary();
}


void ShootState::ShootSecondary()
{
	GetPlayer()->GetSecondarySkill().Shoot();
	EndSecondary();
	GetPlayer()->GetAnimatorComponent().GetController().Trigger("SecondaryShoot");
}

void ShootState::InterruptSecondary()
{
	GetPlayer()->GetSecondarySkill().Interrupt();
	EndSecondary();
}

void ShootState::EndSecondary()
{
	myCombatState = eCombatState::COMBAT_READY;
	ResetCombatTimer();
	GetPlayer()->GetCrosshair().ExitCharge();
	GetPlayer()->GetAnimatorComponent().GetController().SetBool("IsSecondaryCharging", false);
	GetPlayer()->GetLeftArm().DisableArm();
}
