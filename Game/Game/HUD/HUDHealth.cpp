#include "pch.h"
#include "HUDHealth.h"

#include "Engine/Engine.h"

#include "Engine\GameObject\GameObject.h"

#include "Engine/GameObject/Components/SpriteComponent.h"
#include "Engine\GameObject\Components\TextComponent.h"

HUDHealth::HUDHealth(GameObject* aGameObject) : Component(aGameObject)
{
}

HUDHealth::~HUDHealth()
{

}

void HUDHealth::Start()
{
	myHealthSprite  = GetTransform().GetChildByGameObjectName("HealthBar")->GetComponent<Engine::SpriteComponent>();
	myHealthDiffSprite  = GetTransform().GetChildByGameObjectName("HealthDiffBar")->GetComponent<Engine::SpriteComponent>();
	myShieldSprite  = GetTransform().GetChildByGameObjectName("ShieldBar")->GetComponent<Engine::SpriteComponent>();
	myShieldBackground  = GetTransform().GetChildByGameObjectName("ShieldBackground")->GetComponent<Engine::SpriteComponent>();
	myBarrierSprite = GetTransform().GetChildByGameObjectName("BarrierBar")->GetComponent<Engine::SpriteComponent>();

	myHealthText = GetTransform().GetChildByGameObjectName("HealthText")->GetComponent<Engine::TextComponent>();

	myHealthSpriteColor = myHealthSprite->GetColor();
	myShieldSpriteColor = myShieldSprite->GetColor();
	myBarrierSpriteColor = myBarrierSprite->GetColor();
	myShieldBackgroundColor = myShieldBackground->GetColor();
}

void HUDHealth::Execute(Engine::eEngineOrder)
{
	myHealthPercent = CU::Lerp(myHealthPercent, myHealthPercentTarget, 10.f * Time::DeltaTime);

	if (myHealthSprite)
	{
		if(myIsIncreasing)
		{
			myHealthSprite->GetMaterialInstance().SetFloat4("value1", Vec4f(myHealthPercent, 0.0f, 0.0f, 0.0f));
		}
		else
		{
			myHealthSprite->GetMaterialInstance().SetFloat4("value1", Vec4f(myHealthPercentTarget, 0.0f, 0.0f, 0.0f));
		}
	}
	if (myHealthDiffSprite)
	{
		myHealthDiffSprite->GetMaterialInstance().SetFloat4("value1", Vec4f(myHealthPercent, 0.0f, 0.0f, 0.0f));
	}

	if(myHealthPercentTarget <= ourDangerHealthPercentage)
	{
		myFlashTimer -= Time::DeltaTime;
		if(myFlashTimer <= 0.f)
		{
			myFlashTimer = 0.1f;
			myFlashSwitch = !myFlashSwitch;

			myShieldSprite->SetColor({1.f, 1.f, 1.f, 0.f});
			myBarrierSprite->SetColor({1.f, 1.f, 1.f, 0.f});
			myShieldBackground->SetColor({1.f, 1.f, 1.f, 0.f});

			if(myFlashSwitch)
			{
				myHealthSprite->SetColor({1.f, 1.f, 1.f, 1.f});
			}
			else
			{
				myHealthSprite->SetColor({1.f, 0.f, 0.f, 1.f});
			}
		}
	}
	else
	{
		myHealthSprite->SetColor(myHealthSpriteColor);
		myShieldSprite->SetColor(myShieldSpriteColor);
		myBarrierSprite->SetColor(myBarrierSpriteColor);
		myShieldBackground->SetColor(myShieldBackgroundColor);
	}
}

void HUDHealth::SetMaxHealth(const float& aMaxHealth)
{
	myMaxHealth = aMaxHealth;
	CalculateHealth();
}

void HUDHealth::SetHealth(const float& aHealth)
{
	myHealth = aHealth;
	CalculateHealth();
}

void HUDHealth::SetMaxShield(const float& aMaxShield)
{
	SetShields(myShield, aMaxShield);
}

void HUDHealth::SetShield(const float& aShield)
{
	SetShields(aShield, myMaxShield);
}

void HUDHealth::SetShields(const float& aShield, const float& aMaxShield)
{
	myMaxShield = aMaxShield;
	myShield = aShield;

	CalculateHealth();
}

void HUDHealth::SetBarrier(const float& aBarrier)
{
	myBarrier = aBarrier;
	CalculateHealth();
}

void HUDHealth::CalculateHealth()
{
	{
		int maxHP = static_cast<int>(myMaxHealth + myMaxShield + myBarrier);
		int HP = static_cast<int>(myHealth + myShield + myBarrier);
		std::string text = std::to_string(HP) + " / " + std::to_string(maxHP);
		myHealthText->SetText(text);
	}

	float oldPercent = myHealthPercentTarget;
	myHealthPercentTarget = myHealth  / myMaxHealth;
	myIsIncreasing = oldPercent < myHealthPercentTarget;

	float shieldPercent  = myShield  / myMaxHealth;
	float maxShieldPercent = myMaxShield / myMaxHealth;
	float barrierPercent = myBarrier / myMaxHealth;

	if (myShieldSprite)
	{
		myShieldSprite->GetMaterialInstance().SetFloat4("value1", Vec4f(shieldPercent, 0.0f, 0.0f, 0.0f));
	}
	if (myShieldBackground)
	{
		myShieldBackground->GetMaterialInstance().SetFloat4("value1", Vec4f(maxShieldPercent, 0.0f, 0.0f, 0.0f));
	}
	if (myBarrierSprite)
	{
		myBarrierSprite->GetMaterialInstance().SetFloat4("value1", Vec4f(barrierPercent, 0.0f, 0.0f, 0.0f));
	}
}
