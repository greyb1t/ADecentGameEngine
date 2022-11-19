#include "pch.h"
#include "FinalBossFollowGameObject.h"
#include "Engine/GameObject/GameObject.h"

void FB::FinalBossFollowGameObject::Start()
{
}

void FB::FinalBossFollowGameObject::Execute(Engine::eEngineOrder aOrder)
{
	if (GameObject* g = myTargetGameObject.Get())
	{
		const auto& targetTransform = g->GetTransform();

		GetTransform().SetPosition(targetTransform.GetPosition() + myPositionOffset);
	}
}

void FB::FinalBossFollowGameObject::Reflect(Engine::Reflector& aReflector)
{
	aReflector.Reflect(myTargetGameObject, "Target GameObject");
	aReflector.Reflect(myPositionOffset, "Position Offset");
}