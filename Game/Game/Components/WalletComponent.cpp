#include "pch.h"
#include "WalletComponent.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/Reflection/Reflector.h"

#include "HUD\HUDHandler.h"

void WalletComponent::Start()
{
	myHUD = myGameObject->GetComponent<HUDHandler>();
	myHUD->SetMoneyAmmount(GameManager::GetInstance()->GetMoney());
}

bool WalletComponent::TryBuy(int aItemPrice)
{
	if (aItemPrice < 0)
	{
		LOG_INFO(LogType::Markus) << "Item Price Below Zero Gold";
		return false;
	}

	if (aItemPrice <= GameManager::GetInstance()->GetMoney())
	{
		DecreaseGold(aItemPrice);
		LOG_INFO(LogType::Markus) << "Item Price Valid, Gold Lost";
		return true;
	}
	else
	{
		LOG_INFO(LogType::Markus) << "Not Enough Gold";
		return false;
	}
}

void WalletComponent::DecreaseGold(int aAmount)
{
	GameManager::GetInstance()->SetMoney(GameManager::GetInstance()->GetMoney() - aAmount);
	myHUD->SetMoneyAmmount(GameManager::GetInstance()->GetMoney());
}

void WalletComponent::IncreaseGold(int aAmount)
{
	LOG_INFO(LogType::Game) << "Player Picked Up " << aAmount << " Gold";
	GameManager::GetInstance()->SetMoney(GameManager::GetInstance()->GetMoney() + aAmount);
	myHUD->SetMoneyAmmount(GameManager::GetInstance()->GetMoney());
}

void WalletComponent::Reflect(Engine::Reflector& aReflector)
{
	//if(GameManager::GetInstance())
	//{
	//	int32_t amount = GameManager::GetInstance()->GetMoney();
	//	aReflector.Reflect(amount, "Gold Ammount");
	//	GameManager::GetInstance()->SetMoney(amount);
	//}
}