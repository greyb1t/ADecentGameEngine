#include "pch.h"
#include "BodyMouthMortar.h"
#include "AI/FinalBoss/Constants.h"
#include "Engine/GameObject/Components/AnimatorComponent.h"
#include "AI/LevelBoss/Objects/BossMortar.h"
#include "AI/FinalBoss/FinalBossBody.h"
#include "Engine/GameObject/Prefab/GameObjectPrefab.h"
#include "Engine/GameObject/GameObject.h"
#include "AI/FinalBoss/FinalBoss.h"
#include "Engine/GameObject/Components/AudioComponent.h"

FB::BodyMouthMortar::BodyMouthMortar(FinalBossBody& aBody)
	: myBody(aBody)
{
	aBody.GetAnimator().GetController().AddEventCallback("SpawnMortar", [this]() { OnSpawnMortar(); });

	myMouthBoneIndex = aBody.GetAnimator().GetController().GetBoneIndexFromName("CenterJaw");

	aBody.GetAnimator().GetController().AddStateOnExitCallback(
		"Base",
		"Mortar",
		[this]() { myIsFinished = true; });
}

void FB::BodyMouthMortar::Update()
{
}

bool FB::BodyMouthMortar::IsFinished() const
{
	return myIsFinished;
}

void FB::BodyMouthMortar::OnEnter()
{
	// Attack();
	myIsFinished = false;

	myBody.GetAnimator().GetController().Trigger("Mortar");
}

void FB::BodyMouthMortar::OnExit()
{
}

void FB::BodyMouthMortar::Reflect(Engine::Reflector& aReflector)
{
}

void FB::BodyMouthMortar::Attack()
{
	myBody.GetAudio()->PlayEvent("MouthMortar");

	const auto& desc = myBody.GetMortarDesc();

	if (!desc.myMortarPrefab)
	{
		LOG_ERROR(LogType::Game) << "no mortar prefab";
		return;
	}

	Engine::GameObjectPrefab& mortarPrefab = desc.myMortarPrefab->Get();
	GameObject& mortar = mortarPrefab.Instantiate(*myBody.GetGameObject()->GetScene());

	const auto mouthTransform = myBody.GetAnimator().GetController().GetBoneTransformWorld(myMouthBoneIndex);
	Vec3f t;
	Vec3f s;
	Quatf r;
	mouthTransform.Decompose(t, r, s);

	mortar.GetTransform().SetPosition(t);

	if (auto script = mortar.GetComponent<LevelBossStates::BossMortar>())
	{
		script->SetPositions(t, 500.f, desc.myTimeUntilLand);

		script->SetDamage(desc.myDamage.GetDamage());
		script->SetClusterCount(desc.myClusterCount);
		script->SetClusterRadius(desc.myClusterRadius);
		script->SetOffsetLanding(desc.myClusterOffsetLanding);
		script->SetOffsetSpread(desc.myClusterOffsetSpread);
	}
}

void FB::BodyMouthMortar::OnSpawnMortar()
{
	Attack();
}
