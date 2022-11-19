#include "pch.h"
#include "BossDome.h"

#include "Engine/GameObject/GameObject.h"

#include "Engine/GameObject/Components/Collider.h"
#include "Engine/GameObject/Components/ModelComponent.h"
#include "Engine/GameObject/Components/AnimatorComponent.h"

#include "../LevelBoss.h"
#include <Engine/GameObject/Components/MusicManager.h>

void BossDome::Awake()
{
	if (!(myDomeModel = myGameObject->GetComponent<Engine::ModelComponent>()))
	{
		return;
	}

	if (!(myCollider = myGameObject->GetComponent<Engine::Collider>()))
	{
		return;
	}

	const float radius = myGameObject->GetTransform().GetScale().y;
	const float magic_offset = 92.0f;
	myRadius = radius * magic_offset;
	myCollider->SetRadius(myRadius);
}

bool BossDome::Init(LevelBoss* aLevelBoss)
{
	if (!aLevelBoss)
		return false;

	myBoss = aLevelBoss;

	if (!InitComponents())
		return false;

	if (!InitStartValues())
		return false;

	return true;
}

void BossDome::Execute(Engine::eEngineOrder aOrder)
{
	OnUpdateTransitions();

	if (!myBoss)
	{
		if (!myGameObject->IsDestroyed())
		{
			myGameObject->Destroy();
		}
		return;
	}

	if (myBoss->IsFightStarted())
	{
		OnCheckWithinBounds();
	}
}

void BossDome::OnBeginFight()
{
	auto animator = myBoss->GetAnimator();

	myDomeDuration = animator->GetController().FindState("BASE - (Main)", "Spawn")->GetDurationSeconds();
	myDomeProgress = myDomeDuration;
	myState = State::Appear;

	myWasEngaged = true;
}

void BossDome::OnDeath()
{
	myDomeProgress = myDomeDuration;
	myState = State::Disappear;

	myWasEngaged = false;
}

void BossDome::OnHit(RayCastHit& aHit, GameObject* aCollidedWith)
{
	GDebugDrawer->DrawLine3D(DebugDrawFlags::Gameplay, aHit.Position, aHit.Position + aHit.Normal * 1000.0f, 2.0f, Vec4f(1, 0, 0, 1));
}

bool BossDome::InitComponents()
{
	if (!myDomeModel)
	{
		LOG_ERROR(LogType::Game) << "No model component was found on the dome object!";
		return false;
	}

	if (myDomeModel->GetMeshInstances().empty())
	{
		LOG_ERROR(LogType::Game) << "No mesh instances were found on the dome object!";
		return false;
	}

	if (!myCollider)
	{
		LOG_ERROR(LogType::Game) << "No collider component was found on the dome object!";
		return false;
	}

	return true;
}

bool BossDome::InitStartValues()
{
	myDomeProgress = 0.0f;
	Vec4f val = myDomeModel->GetMeshMaterialInstanceByIndex(0).GetFloat4("myVar");
	val.z = 0.0f;
	myDomeModel->GetMeshMaterialInstanceByIndex(0).SetFloat4("myVar", val);

	return true;
}

void BossDome::OnUpdateTransitions()
{
	if (myDomeProgress > 0.0f)
	{
		myDomeProgress -= Time::DeltaTime;
		if (myDomeProgress <= 0.0f)
		{
			myDomeProgress = 0.0f;
		}

		float ratio = 0.0f;

		if (myState == State::Appear)
		{
			ratio = 1.0f - (myDomeProgress / myDomeDuration);
		}
		else
		{
			ratio = myDomeProgress / myDomeDuration;
		}

		myDomeModel->GetMeshMaterialInstanceByIndex(0).SetAlpha(ratio);

		Vec4f val = myDomeModel->GetMeshMaterialInstanceByIndex(0).GetFloat4("myVar");
		val.z = ratio;
		myDomeModel->GetMeshMaterialInstanceByIndex(0).SetFloat4("myVar", val);
	}
}

void BossDome::OnCheckWithinBounds()
{
	auto player = myBoss->GetPlayerObject();
	const auto& playerPos = player->GetTransform().GetPosition();
	const auto& bossPos = myBoss->GetTransform().GetPosition();

	const bool isWithinBounds = (playerPos - bossPos).LengthSqr() < myRadius * myRadius;

	OnToggleCollision(isWithinBounds);
	OnToggleEngagement(isWithinBounds);
}

void BossDome::OnToggleCollision(const bool aIsWithinBounds)
{
	Layer layer = (!aIsWithinBounds) ? eLayer::PLAYER_PROJECTILE_BLOCKING : eLayer::NONE;

	myGameObject->SetLayer(layer);
}

void BossDome::OnToggleEngagement(const bool aIsWithinBounds)
{
	if (myWasEngaged != aIsWithinBounds)
	{
		aIsWithinBounds ? myBoss->Engage() : myBoss->Disengage();
	}
	myWasEngaged = aIsWithinBounds;

	if (!aIsWithinBounds && !myBoss->IsDisengaged())
	{
		myBoss->Disengage();
	}
}
