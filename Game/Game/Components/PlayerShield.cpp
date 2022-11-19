#include "pch.h"
#include "PlayerShield.h"
#include "Engine\GameObject\GameObject.h"
#include "Engine\GameObject\Components\Collider.h"
#include "Engine\ResourceManagement\Resources\AnimationCurveResource.h"
#include "Engine/GameObject/Components/AudioComponent.h"
#include "Components\HealthComponent.h"

void PlayerShield::Reflect(Engine::Reflector& aReflector)
{
	aReflector.Reflect(myDefaultRadius, "Radius");
	aReflector.Reflect(mySizeCurveRef, "Size Curve");
	aReflector.Reflect(myLifeTime, "LifeTime");
}

void PlayerShield::Awake()
{
	//myGameObject->SetLayer(eLayer::PLAYER_SHIELD);

	myCollider = myGameObject->GetComponent<Engine::Collider>();
	myCollider->SetRadius(myDefaultRadius);

	if(mySizeCurveRef && mySizeCurveRef->IsValid())
		mySizeCurve = mySizeCurveRef->Get();

	myAudio = myGameObject->AddComponent<Engine::AudioComponent>();
	{
		myAudio->AddEvent("Close", "event:/SFX/PLR/ShieldPowerDown");
	}
}

void PlayerShield::Start()
{
	myPlayer = &Main::GetPlayer()->GetTransform();

	myShieldModel = &GetTransform().GetChildByGameObjectName("Model")->GetTransform();
	myModelStartScale = myShieldModel->GetScale().x;

	Main::AddPlayerShootObserver(myID, [&]() { Deactivate(); });

	if (!Main::GetCanOpenChestShield())
	{
		Deactivate(true);
	}

	myPlayerHealth = myPlayer->GetGameObject()->GetComponent<HealthComponent>();
}

void PlayerShield::Execute(Engine::eEngineOrder aOrder)
{
	if (IsPlayerInRadius() && myPlayerHealth)
	{
		LOG_INFO(LogType::Simon) << "IS IN SHIELD";
		myPlayerHealth->SetIsInShield(true);
	}

	if (myIsActive)
	{
		ActiveUpdate();
	}
	else
	{
		UnActiveUpdate();
	}


	//GDebugDrawer->DrawSphere3D(DebugDrawFlags::Always, GetTransform().GetPosition(), myRadius);
}

void PlayerShield::Deactivate(bool aInstantly)
{
	if (!myIsActive)
		return;

	myIsActive = false;

	if (myDeactivateCallback)
		myDeactivateCallback();

	if (aInstantly)
		myDeactivateTimer = myDeactivateDelay;

	myGameObject->SetLayers(eLayer::NONE, eLayer::NONE, eLayer::NONE);
}

bool PlayerShield::IsPlayerInRadius()
{
	float distanceToPlayer = (myPlayer->GetPosition() - GetTransform().GetPosition()).Length();

	return distanceToPlayer < myRadius;
}

bool PlayerShield::IsPlayerInMaxRadius()
{
	float distanceToPlayer = (myPlayer->GetPosition() - GetTransform().GetPosition()).Length();

	return distanceToPlayer < myDefaultRadius;
}

void PlayerShield::SetDeactivateCallback(const std::function<void()>& aCallback)
{
	myDeactivateCallback = aCallback;
}

void PlayerShield::KnockBackEnemy()
{
	float height = 500;
	float force = 13.f;

	std::vector<OverlapHit> hits;

	if (myGameObject->GetScene()->SphereCastAll(myGameObject->GetTransform().GetPosition(), myRadius, eLayer::ENEMY, hits))
	{
		for (int hitIndex = 0; hitIndex < hits.size(); hitIndex++)
		{
			Vec3f knockBackDirection = hits[hitIndex].GameObject->GetTransform().GetPosition() - myGameObject->GetTransform().GetPosition();
			knockBackDirection.y += height;
			knockBackDirection.Normalize();
			knockBackDirection *= force;

			hits[hitIndex].GameObject->GetComponent<HealthComponent>()->ApplyKnockback(knockBackDirection);
		}
	}
}

void PlayerShield::ActiveUpdate()
{
	UpdateRadius(true);

	//if(myPlayerComp->GetStatus().isAiming) //Temp lösning, gör callback
	//	Deactivate(false);

	if(!IsPlayerInMaxRadius())
		Deactivate(true);

	myLifeTimer += Time::DeltaTime;
	if (myLifeTimer > myLifeTime)
		Deactivate();

	KnockBackEnemy();
}

void PlayerShield::UnActiveUpdate()
{
	myDeactivateTimer += Time::DeltaTime;

	if (myDeactivateTimer < myDeactivateDelay)
		return;

	if (!myHasPlayerClose)
	{
		myHasPlayerClose = true;
		myAudio->PlayEvent("Close");
	}

	UpdateRadius(false);

	if (myRadius <= 0.0f)
	{
		myPlayerHealth->SetIsInShield(false);
		myGameObject->Destroy();
		Main::RemovePlayerShootObserver(myID);
	}
}

void PlayerShield::UpdateRadius(bool aShouldGrow)
{
	myResizeTimer += aShouldGrow ? Time::DeltaTime : -Time::DeltaTime * 1.5f;
	myResizeTimer = std::clamp(myResizeTimer, 0.0f, myResizeTime);

	float percent = myResizeTimer / myResizeTime;
	float curveVal = mySizeCurve.Evaluate(percent);

	myRadius = curveVal * myDefaultRadius;
	float scale = curveVal * myModelStartScale;

	//myCollider->SetRadius(myRadius);
	myShieldModel->SetScale(scale);
}
