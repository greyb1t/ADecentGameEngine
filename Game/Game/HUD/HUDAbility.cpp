#include "pch.h"
#include "HUDAbility.h"

#include "Engine/Engine.h"

#include "Engine/GameObject/Components/SpriteComponent.h"
#include "Engine/GameObject/Components/TextComponent.h"

#include "Engine\Reflection\Reflector.h"
#include "Engine\GameObject\GameObject.h"

HUDAbility::HUDAbility(GameObject* aGameObject) : Component(aGameObject)
{
}

HUDAbility::~HUDAbility()
{

}

void HUDAbility::Start()
{
	myAbilityIcon = GetTransform().GetChildByGameObjectName("Icon")->GetComponent<Engine::SpriteComponent>();
	myCooldownOverlay = GetTransform().GetChildByGameObjectName("Overlay")->GetComponent<Engine::SpriteComponent>();
	myEffect = GetTransform().GetChildByGameObjectName("Effect")->GetComponent<Engine::SpriteComponent>();
	myTimerText = GetTransform().GetChildByGameObjectName("Timer")->GetComponent<Engine::TextComponent>();

	myCooldownOverlay->SetActive(false);
}

void HUDAbility::Reflect(Engine::Reflector& aReflector)
{
	aReflector.Reflect(myCooldownTime, "Cooldown Time");
}

void HUDAbility::Execute(Engine::eEngineOrder aOrder)
{
	float lastTime = myTimer;
	myTimer -= Time::DeltaTime;

	myTimer = C::Max(0.0f, myTimer);

	if(myTimer != lastTime)
	{
		float percentage = myTimer / myCooldownTime;

		myCooldownOverlay->SetActive(myTimer != 0.0f);

		int time = static_cast<int>(std::floorf(myTimer)) + 1;
		myTimerText->SetText(std::to_string(time));
		myTimerText->SetActive(myTimer > 0.0f);

		if (myTimer == 0.0f)
		{
			myEffectTimer = 0.0f;
			myEffect->GetGameObject()->SetActive(true);
			myEffect->GetGameObject()->GetTransform().SetScale(1.0f);
		}
	}

	if (myEffect->IsActive())
	{
		myEffectTimer += Time::DeltaTime;

		if (myEffectTimer > myEffectTime)
		{
			myEffect->GetGameObject()->SetActive(false);
		}
		else
		{
			float percentage = myEffectTimer / myEffectTime;
			percentage = 1.0f - percentage;
			percentage = std::max(percentage, 0.0f);

			myEffect->GetGameObject()->GetTransform().SetScale(percentage);
		}
	}
}

void HUDAbility::StartCooldown()
{
	myTimer = myCooldownTime;
}

void HUDAbility::ResetCooldown()
{
	myTimer = 0.0f;
	myEffect->GetGameObject()->SetActive(false);
}

void HUDAbility::SetCooldownTime(const float& aTime)
{
	myCooldownTime = aTime;
}
