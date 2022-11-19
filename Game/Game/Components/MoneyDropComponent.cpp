#include "pch.h"
#include "MoneyDropComponent.h"

#include "Engine/Reflection/Reflector.h"
#include "Engine/GameObject/GameObject.h"

#include "Engine/ResourceManagement/Resources/VFXResource.h"
#include "Engine/GameObject/Components/VFXComponent.h"
#include "Game/Components/HealthComponent.h"

#include "WalletComponent.h"

void MoneyDropComponent::Start()
{
	{
		if (myMoneyRef->IsValid())
		{
			myMoneyVFX = myGameObject->AddComponent<Engine::VFXComponent>(myMoneyRef->Get());
		}
		else
		{
			LOG_ERROR(LogType::Game) << "Money Drop Component is missing VFX";
		}
	}

	{
		myHealthComponent = myGameObject->GetComponent<HealthComponent>();

		if (!myHealthComponent)
			myHealthComponent = myGameObject->AddComponent<HealthComponent>();

		myHealthComponent->ObserveDeath([&](float aDmg) { OnDeath(); });
	}
}

void MoneyDropComponent::Reflect(Engine::Reflector& aReflector)
{
	aReflector.Reflect(myMoneyDrop, "Money Drop Ammount");
	aReflector.Reflect(myMoneyRef, "Money VFX");
}

void MoneyDropComponent::SetMoneyDropAmmount(int aAmmount)
{
	myMoneyDrop = aAmmount;
}

void MoneyDropComponent::ZeroMoneyDrop()
{
	myMoneyDrop = 0;
}

void MoneyDropComponent::OnDeath()
{
	//DROP GOLD
	auto player = Main::GetPlayer();
	if (player)
	{
		auto wallet = player->GetComponent<WalletComponent>();

		if (wallet)
		{
			wallet->IncreaseGold(myMoneyDrop);
		}
		else
		{
			LOG_ERROR(LogType::Game) << "MoneyDropComponent couldn't find the player wallet!";
		}
	}

	//PLAY VFX
	if (myMoneyVFX)
	{
		//myMoneyVFX->Play();
	}
}
