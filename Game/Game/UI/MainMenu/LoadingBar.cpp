#include "pch.h"
#include "LoadingBar.h"

#include "Engine/GameObject/GameObject.h"
#include "Engine/GameObject/Components/TextComponent.h"
#include "Engine/GameObject/Components/SpriteComponent.h"

#include "Engine/Scene/Scene.h"
#include "Engine/Scene/SceneManager.h"

#include <iomanip>
#include <sstream>

void LoadingBar::Awake()
{
	if (mySprite = myGameObject->GetComponent<Engine::SpriteComponent>())
	{
		auto& instance = mySprite->GetMaterialInstance();
		instance.SetFloat4("value1", { 0, 0, 0, 0 });
	}

	if (!GetTransform().GetChildren().empty())
	{
		myText = GetTransform().GetChildByIndex(0)->GetGameObject()->GetComponent<Engine::TextComponent>();
	}
}

void LoadingBar::Execute(Engine::eEngineOrder aOrder)
{
	auto& sm = GetEngine().GetSceneManager();
	if (sm.GetCurrentLoadJob().has_value())
	{
		auto& job = sm.GetCurrentLoadJob().value();
		const float percentage = job.myScene->GetLoadingPercent();

		if (mySprite)
		{
			auto& instance = mySprite->GetMaterialInstance();

			const float old = instance.GetFloat4("value1").x;
			float value = CU::Lerp(old, percentage, 7.5f * Time::DeltaTimeUnscaled);
			if (value > old + 0.0001f)
			{
				value = CU::Clamp(old, percentage, value);
				instance.SetFloat4("value1", { value, 0, 0, 0 });
			}
		}

		if (myText)
		{
			std::stringstream ss;
			ss << std::fixed << std::setprecision(0) << (percentage * 100.0f) << " %";
			std::string s = ss.str();
			myText->SetText(s);
		}
	}
}
