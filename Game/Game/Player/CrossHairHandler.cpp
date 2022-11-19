#include "pch.h"
#include "CrossHairHandler.h"
#include "Player/Player.h"
#include "HUD/CrosshairUI.h"
#include "HUD/HUDHandler.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/ResourceManagement/Resources/AnimationCurveResource.h"
#include "Engine/AnimationCurve/Curve.h"

CrossHairHandler::CrossHairHandler(Player* aPlayer)
	: myPlayer(aPlayer)
{

}

void CrossHairHandler::Update()
{
	if (myHitmark.active)
		UpdateHitMark();

	if (myState == eCrosshairState::SECONDARY_CHARGE || myState == eCrosshairState::SECONDARY_RETURN)
		UpdateCharge();

	const float steadiness = myPlayer->GetSteadiness().Get();

	const float distance = myInner + mySteadinessOffset * myPlayer->GetSteadiness().Get();

	// NOTE(filip): fix crash
	if (myPlayer->GetHUDHandler()->GetCrosshairUI())
	{
		myPlayer->GetHUDHandler()->GetCrosshairUI()->MoveLine(0, myRotation + 0, distance);
		myPlayer->GetHUDHandler()->GetCrosshairUI()->MoveLine(1, myRotation + 90, distance);
		myPlayer->GetHUDHandler()->GetCrosshairUI()->MoveLine(2, myRotation + 180, distance);
		myPlayer->GetHUDHandler()->GetCrosshairUI()->MoveLine(3, myRotation + 270, distance);
	}
}

void CrossHairHandler::HitMark()
{
	myHitmark.active = true;
	myHitmark.timer = 0;
	UpdateHitMarkSprite(0);
}

void CrossHairHandler::EnterCharge()
{
	myState = eCrosshairState::SECONDARY_CHARGE;
	mySecondary.timer = 0;
}

void CrossHairHandler::ExitCharge()
{
	if (myState == eCrosshairState::SECONDARY_CHARGE)
		myState = eCrosshairState::SECONDARY_RETURN;

	mySecondary.timer = 0;
	mySecondary.returnRotation = myRotation;
	if (myRotation < 180.f)
	{
		myRotation += 180;
	}
	//myRotation = 0;
}

float& CrossHairHandler::RotationRef()
{
	return myRotation;
}

float& CrossHairHandler::InnerRef()
{
	return myInner;
}

float& CrossHairHandler::SteadinessOffsetRef()
{
	return mySteadinessOffset;
}

float& CrossHairHandler::ChargeRotationSpeedRef()
{
	return myChargeSpeed;
}

void CrossHairHandler::UpdateCharge()
{
	if (myState == eCrosshairState::SECONDARY_CHARGE) 
	{
		if (myRotation > 360.f * 2.f)
			myRotation -= 360.f;

		mySecondary.timer += Time::DeltaTime;

		float percentage = std::min(mySecondary.timer / myPlayer->GetSkillStats().secondary.chargeTime, 1.f);

		float speed = myChargeSpeed;
		auto speedLerp = myPlayer->GetSettings().crossHair.secondary.rotationSpeedCurve;
		if (speedLerp && speedLerp->IsValid())
		{
			speed = speedLerp->Get().Evaluate(percentage) * myChargeSpeed;
		}

		myRotation += speed * Time::DeltaTime;
	}
	
	if (myState == eCrosshairState::SECONDARY_RETURN)
	{
		mySecondary.timer += Time::DeltaTime;
		float percentage = mySecondary.timer / myPlayer->GetSettings().crossHair.secondary.returnTime;
		if (percentage >= 1.f) {
			myRotation = 0;
			myState = eCrosshairState::SHOOT;
			return;
		}

		auto returnLerp = myPlayer->GetSettings().crossHair.secondary.returnLerp;
		if (returnLerp && returnLerp->IsValid())
		{
			myRotation = mySecondary.returnRotation * returnLerp->Get().Evaluate(percentage);
		}
	}

}

void CrossHairHandler::UpdateHitMark()
{
	if (!myHitmark.active)
		return;

	myHitmark.timer += Time::DeltaTime;

	const auto settings = myPlayer->GetSettings().crossHair.hitmark;

	if (myHitmark.timer >= settings.timeActive)
	{
		EndHitMark();
		return;
	}

	// Update scale and alpha
	UpdateHitMarkSprite(myHitmark.timer / settings.timeActive);
}

void CrossHairHandler::EndHitMark()
{
	myHitmark.active = false;
	myPlayer->GetHUDHandler()->GetCrosshairUI()->SetHitMark(false);
}

void CrossHairHandler::UpdateHitMarkSprite(float aLifeTimePercentage)
{
	const auto settings = myPlayer->GetSettings().crossHair.hitmark;
	float size = (settings.sizeCurve && settings.sizeCurve->IsValid()) ? settings.sizeCurve->Get().Evaluate(aLifeTimePercentage) : aLifeTimePercentage;
	float alpha = (settings.alphaCurve && settings.alphaCurve->IsValid()) ? settings.alphaCurve->Get().Evaluate(aLifeTimePercentage) : aLifeTimePercentage;

	myPlayer->GetHUDHandler()->GetCrosshairUI()->SetHitMark(true, size, alpha);
}
