#include "pch.h"
#include "GraphNode_FadeAllChildren.h"
#include "Engine/GraphManager/Nodes/Base/GraphNodeInstance.h"
#include "Engine/GameObject/Components/SpriteComponent.h"
#include "Engine/GameObject/Components/ScriptsComponent.h"
#include "Engine/GameObject/GameObject.h"
#include <Engine/GameObject/Components/2DComponents/ButtonComponent.h>

GraphNode_FadeAllChildren::GraphNode_FadeAllChildren()
{
	CreatePin("IN", PinDirection::PinDirection_IN, DataType::Exec);

	CreatePin("Speed", PinDirection::PinDirection_IN, DataType::Float);

	CreatePin("OUT", PinDirection::PinDirection_OUT, DataType::Exec);
}

int GraphNode_FadeAllChildren::OnExec(class GraphNodeInstance* aNodeInstance)
{
	float speed = GetPinData<float>(aNodeInstance, 1);
	GameObject* thisGo = aNodeInstance->GetGameObject();
	FadeChildren(thisGo, speed, aNodeInstance);

	return 2;
}

void GraphNode_FadeAllChildren::FadeChildren(GameObject* aGo, float aSpeed, class GraphNodeInstance* aNodeInstance)
{
	float maxAlpha = 0;
	for (Transform* goTrans : aGo->GetTransform().GetChildren())
	{
		FadeChildren(goTrans->GetGameObject(), aSpeed, aNodeInstance);

		auto* sprite = goTrans->GetGameObject()->GetComponent<Engine::SpriteComponent>();
		if (sprite == nullptr)
			continue;

		/*	auto* script = goTrans->GetGameObject()->GetComponent<Engine::ScriptComponent>();

			if (script == nullptr)
				continue;*/
		if (sprite->GetAlpha() > 0.001f)
		{
			sprite->SetAlpha(sprite->GetAlpha() - aSpeed);
			if (sprite->GetAlpha() < 0.0f)
			{
				sprite->SetAlpha(0.0f);
			}
			if (sprite->GetAlpha() > 1.0f)
			{
				sprite->SetAlpha(1.0f);
			}
			if (sprite->GetAlpha() > maxAlpha)
			{
				maxAlpha = sprite->GetAlpha();
			}
		}

		auto* button = goTrans->GetGameObject()->GetComponent<Engine::ButtonComponent>();
		if (button == nullptr)
			continue;
		
		button->SetActive(false);

		auto* script = goTrans->GetGameObject()->GetComponent<Engine::ScriptsComponent>();
		if (script == nullptr)
			continue;

		script->SetActive(false);

	}
}
