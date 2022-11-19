#include "pch.h"
#include "HUDHandler.h"

#include "Engine/Engine.h"
#include "Engine\Reflection\Reflector.h"
#include "Engine/GameObject/GameObject.h"

#include "Engine\GameObject\Components\TextComponent.h"
#include "Engine\UIEventManager\UIEventManager.h"
#include "Items\ItemManager.h"
#include "Camera\CameraController.h"

#include "HUDAbility.h"
#include "HUDHealth.h"
#include "HUDItemDescription.h"
#include "HUDInteraction.h"
#include "HUDDifficulty.h"
#include "CrosshairUI.h"
#include "Game/Player/Player.h"

HUDHandler::HUDHandler(GameObject* aGameObject) : Component(aGameObject)
{
}

HUDHandler::~HUDHandler()
{
	UIEventManager::RemoveFadeOutObserver();
}

void HUDHandler::Reflect(Engine::Reflector& aReflector)
{

}

void HUDHandler::Awake()
{
	GameObject* hudHolder = GetTransform().GetParent()->GetChildByGameObjectName("HUD");

	if (!hudHolder)
		ErrorAndDisable();

	Transform& hudTransform = hudHolder->GetTransform();
	Transform& abilityTransform = hudTransform.GetChildByGameObjectName("Abilities")->GetTransform();

	if (!SetAbility(abilityTransform, "PrimaryFire", myPrimaryFire))
		ErrorAndDisable();

	if (!SetAbility(abilityTransform, "SecondaryFire", mySecondaryFire))
		ErrorAndDisable();

	if (!SetAbility(abilityTransform, "Mobility", myMobility))
		ErrorAndDisable();

	if (!SetAbility(abilityTransform, "Utiliy", myUtiliy))
		ErrorAndDisable();


	{//Objective
		GameObject* object = hudTransform.GetChildByGameObjectName("Objective")->GetTransform().GetChildByGameObjectName("Text");

		if (object)
		{
			Engine::TextComponent* text = object->GetComponent<Engine::TextComponent>();

			if (text)
			{
				myObjective = text;
			}
			else
			{
				ErrorAndDisable();
			}
		}
		else
		{
			ErrorAndDisable();
		}
	}


	{ //Health
		GameObject* object = hudTransform.GetChildByGameObjectName("Health");

		if (object)
		{
			HUDHealth* health = object->GetComponent<HUDHealth>();

			if (health)
			{
				myHealth = health;
			}
			else
			{
				ErrorAndDisable();
			}
		}
		else
		{
			ErrorAndDisable();
		}
	}

	{ //Money
		GameObject* object = hudTransform.GetChildByGameObjectName("Money");

		if (object)
		{
			GameObject* textObject = object->GetTransform().GetChildByGameObjectName("MoneyText");

			if (textObject)
			{
				myMoneyAmmount = textObject->GetComponent<Engine::TextComponent>();
			}
			else
			{
				ErrorAndDisable();
			}
		}
		else
		{
			ErrorAndDisable();
		}
	}

	{ //Item Pickup Desc
		GameObject* object = hudTransform.GetChildByGameObjectName("PickupInfo");

		if (object)
		{
			HUDItemDescription* itemDesc = object->GetComponent<HUDItemDescription>();

			if (itemDesc)
			{
				myItemDescription = itemDesc;
			}
			else
			{
				ErrorAndDisable();
			}
		}
		else
		{
			ErrorAndDisable();
		}
	}

	{ //Interaction
		GameObject* object = hudTransform.GetChildByGameObjectName("Interaction");

		if (object)
		{
			HUDInteraction* itemDesc = object->GetComponent<HUDInteraction>();

			if (itemDesc)
			{
				myInteraction = itemDesc;
			}
			else
			{
				ErrorAndDisable();
			}
		}
		else
		{
			ErrorAndDisable();
		}
	}
	
	{ //HUD Difficulty
		GameObject* object = hudTransform.GetChildByGameObjectName("TimeDifficulty");

		if (object)
		{
			HUDDifficulty* difficulty = object->GetComponent<HUDDifficulty>();

			if (difficulty)
			{
				myHUDDifficulty = difficulty;
			}
			else
			{
				ErrorAndDisable();
			}
		}
		else
		{
			ErrorAndDisable();
		}
	}


	{ //Crosshair
		GameObject* object = hudTransform.GetChildByGameObjectName("Crosshair");
		if (object)
		{
			if (!(myCrosshair = object->GetComponent<CrosshairUI>())) 
			{
				ErrorAndDisable();
			}
		}
		else
		{
			ErrorAndDisable();
		}
	}

	{//Death Screen
		myDeathScreen = hudTransform.GetChildByGameObjectName("DeathScreen");
		if (!myDeathScreen)
			ErrorAndDisable();
	}
	
	{//Fade Sprite
		myFade = hudTransform.GetChildByGameObjectName("SceneFade")->GetComponent<Engine::SpriteComponent>();
		myFade->SetAlpha(1.0f);
		myFadeTimer = myFadeTime;// 0.0f;
	}

	UIEventManager::SetObjectiveObserver([&](const std::string& aObjective) { SetObjective(aObjective); });
}

void HUDHandler::Start()
{
	UIEventManager::AddEvent("DeathRetry", [&](GameObject* aButton) { DeathRetry(aButton); });
	UIEventManager::AddEvent("DeathMainMenu", [&](GameObject* aButton) { DeathMainMenu(aButton); });

	UIEventManager::SetFadeOutObserver([&](const std::function<void()>& aCallback, bool aShouldFadeBackIn) { FadeOut(aCallback, aShouldFadeBackIn); });
}


void HUDHandler::Execute(Engine::eEngineOrder aOrder)
{
	mySpawnBlackTimer += Time::DeltaTimeUnscaled;
	if (mySpawnBlackTimer > mySpawnBlackTime)
	{
		myFadeTimer += myIsBlack ? Time::DeltaTimeUnscaled : -Time::DeltaTimeUnscaled * 0.4f;
		myFadeTimer = std::clamp(myFadeTimer, 0.0f, myFadeTime);

		float alpha = myFadeTimer / myFadeTime;
		myFade->SetAlpha(alpha);

		if (myIsBlack && alpha == 1.0f)
		{
			if (myShouldFadeBackIn)
			{
				myShouldFadeBackIn = false;
				myIsBlack = !myIsBlack;
			}

			if (!myHasDoneCallback)
			{
				myHasDoneCallback = true;

				if (myFadeOutCallback)
					myFadeOutCallback();
			}
		}
	}


	if (myHasDied)
	{
		myDeathTimer += Time::DeltaTimeUnscaled;

		if (myDeathTimer > myDeathDelay && !myDeathScreenIsOpen)
		{
			myDeathScreenIsOpen = true;
			ActuallyActivateDeathScreen();
		}
	}
}

void HUDHandler::StartCooldown(AbilityType aAbility)
{
	switch (aAbility)
	{
	case HUDHandler::AbilityType::PrimaryFire:
		myPrimaryFire->StartCooldown();
		break;

	case HUDHandler::AbilityType::SecondaryFire:
		mySecondaryFire->StartCooldown();
		break;

	case HUDHandler::AbilityType::Mobility:
		myMobility->StartCooldown();
		break;

	case HUDHandler::AbilityType::Utiliy:
		myUtiliy->StartCooldown();
		break;
	}
}

void HUDHandler::ResetAllCooldowns()
{
	myPrimaryFire->ResetCooldown();

	mySecondaryFire->ResetCooldown();

	myMobility->ResetCooldown();

	myUtiliy->ResetCooldown();
}

HUDAbility* HUDHandler::GetAbility(AbilityType aAbility)
{
	switch (aAbility)
	{
	case HUDHandler::AbilityType::PrimaryFire:
		return myPrimaryFire;

	case HUDHandler::AbilityType::SecondaryFire:
		return mySecondaryFire;

	case HUDHandler::AbilityType::Mobility:
		return myMobility;

	case HUDHandler::AbilityType::Utiliy:
		return myUtiliy;

	default: return nullptr;
	}
}

HUDHealth* HUDHandler::GetHealth()
{
	return myHealth;
}

HUDItemDescription* HUDHandler::GetItemDescriptionBox()
{
	return myItemDescription;
}

HUDInteraction* HUDHandler::GetInteractionBox()
{
	return myInteraction;
}

CrosshairUI* HUDHandler::GetCrosshairUI()
{
	return myCrosshair;
}

void HUDHandler::SetMoneyAmmount(int aMoney)
{
	myMoneyAmmount->SetText(std::to_string(aMoney));
}

void HUDHandler::IterateStageNumber()
{
	myHUDDifficulty->IterateStageNumber();
}

void HUDHandler::ActivateDeathScreen()
{
	myHasDied = true;

}

void HUDHandler::SetObjective(const std::string& aObjective)
{
	myObjective->SetText(aObjective);
}

void HUDHandler::FadeOut(const std::function<void()>& aCallback, bool aShouldFadeBackIn)
{
	myIsBlack = true;
	myHasDoneCallback = false;
	myFadeOutCallback = aCallback;

	myShouldFadeBackIn = aShouldFadeBackIn;
}

void HUDHandler::SetCrosshairVisible(bool anIsVisible)
{
	myCrosshair->SetVisible(anIsVisible);
}

bool HUDHandler::SetAbility(Transform& aToSearchFrom, const std::string& aGameobjectName, HUDAbility*& aObjectToAssign)
{
	GameObject* object = aToSearchFrom.GetChildByGameObjectName(aGameobjectName);

	if (!object)
		return false;

	HUDAbility* ability = object->GetComponent<HUDAbility>();

	if(!ability)
		return false;

	aObjectToAssign = ability;

	return true;
}

void HUDHandler::ErrorAndDisable()
{
	LOG_ERROR(LogType::Game) << "PLAYER HUD FAILED TO INIT! ASK SIMON FOR HELP";
	SetActive(false);
}

void HUDHandler::DeathRetry(GameObject* aButton)
{
	UIEventManager::TryToFadeOut([&]()
	{
		if (auto player = Main::GetPlayer())
		{
			if (auto comp = player->GetComponent<Player>())
			{
				comp->ActivateLoadingScreen();
			}
		}

		Time::PauseTimeScale = 1.0f;
		GameManager::GetInstance()->SetShouldUpdateGameTime(true);
		GameManager::GetInstance()->Reset();
		GameManager::GetInstance()->ChangeLevel();
		Main::CueResetPlayer();
	}, true);
}

void HUDHandler::DeathMainMenu(GameObject* aButton)
{
	UIEventManager::TryToFadeOut([&]()
	{
		Time::PauseTimeScale = 1.0f;
		GameManager::GetInstance()->SetShouldUpdateGameTime(true);
		GameManager::GetInstance()->Reset();
		GameManager::GetInstance()->ChangeToMainMenu();
	});
}

void HUDHandler::ActuallyActivateDeathScreen()
{
	Time::PauseTimeScale = 1.0f;
	GameManager::GetInstance()->SetShouldUpdateGameTime(false);

	myDeathScreen->GetTransform().SetPosition(Vec3f());

	auto& input = GetEngine().GetInputManager();
	input.SetMouseWrapping(false);

	auto camera = Main::GetPlayer()->GetComponent<CameraController>();
	camera->SetIscontrollingCamera(false);
}
