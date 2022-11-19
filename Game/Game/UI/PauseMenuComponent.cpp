#include "pch.h"
#include "PauseMenuComponent.h"

#include "Engine/Engine.h"

#include "Engine/GameObject/Components/SpriteComponent.h"
#include "Engine/GameObject/Components/TextComponent.h"
#include "Engine/GameObject/Components/2DComponents/ButtonComponent.h"
#include "Engine/GameObject/Components/AudioComponent.h"

#include "Engine\Reflection\Reflector.h"
#include "Engine\GameObject\GameObject.h"

#include "Camera\CameraController.h"
#include "Components\InventoryComponent.h"
#include "HUD\HUDItemDescription.h"

#include "Engine\UIEventManager\UIEventManager.h"
#include "Game\GameManager\GameManager.h"
#include "Engine/GameObject/Prefab/GameObjectPrefab.h"

#include "Components/HealthComponent.h"
#include "Player/Player.h"

PauseMenuComponent::PauseMenuComponent(GameObject* aGameObject) : Component(aGameObject)
{

}

PauseMenuComponent::~PauseMenuComponent()
{

}

void PauseMenuComponent::Start()
{
	GameObject* playerObj = Main::GetPlayer();
	myPlayer = playerObj->GetComponent<Player>();

	myItemBox = GetTransform().GetChildByGameObjectName("Item Box")->GetComponent<Engine::SpriteComponent>();
	myBackground = GetTransform().GetChildByGameObjectName("Background")->GetComponent<Engine::SpriteComponent>();

	myFirstItemHover.mySprite = GetTransform().GetChildByGameObjectName("ItemHover")->GetComponent<Engine::SpriteComponent>();
	mySecondItemHover.mySprite = GetTransform().GetChildByGameObjectName("ItemHover2")->GetComponent<Engine::SpriteComponent>();

	myCurrentItemHover = &myFirstItemHover;

	myItemDescription = GetTransform().GetParent()->GetChildByGameObjectName("PauseItemDesc")->GetComponent<HUDItemDescription>();

	myCamera = GetTransform().GetParent()->GetParent()->GetChildByGameObjectName("Player")->GetComponent<CameraController>();

	myStatsText = GetTransform().GetChildByGameObjectName("Stats")->GetComponent<Engine::TextComponent>();

	myResumeButton = GetTransform().GetChildByGameObjectName("Resume");
	mySettingsButton = GetTransform().GetChildByGameObjectName("Settings");
	myQuitButton = GetTransform().GetChildByGameObjectName("Main Menu");

	myAreYouSure = GetTransform().GetChildByGameObjectName("AreYouSure");

	myResumeButton->SetActive(false);
	mySettingsButton->SetActive(false);
	myQuitButton->SetActive(false);

	UIEventManager::AddEvent("PauseResume", [&](GameObject* aButton) { Resume(aButton); });
	UIEventManager::AddEvent("PauseSettings", [&](GameObject* aButton) { Settings(aButton); });
	UIEventManager::AddEvent("PauseQuit", [&](GameObject* aButton) { Quit(aButton); });

	UIEventManager::AddEvent("PauseSureYes", [&](GameObject* /*aButton*/) { SureYes(); });
	UIEventManager::AddEvent("PauseSureNo", [&](GameObject* /*aButton*/) { SureNo(); });

	myItemBox->SetActive(myIsPaused);
	myBackground->SetActive(myIsPaused);

	Time::PauseTimeScale = 1.0f;


	if (mySettingsRef->IsValid())
	{
		Engine::GameObjectPrefab& settingsPrefab = mySettingsRef->Get();
		mySettings = &settingsPrefab.Instantiate(*myGameObject->GetScene());
		mySettings->GetTransform().SetParent(&GetTransform());
		mySettings->GetTransform().SetPosition(Vec3f(0.0f, 1.0f, 0.0f));
		
		UIEventManager::AddEvent("BackToPauseMenu", [&](GameObject* aButton) { Settings(aButton); });
		//Get button event "BackToPauseMenu" till Settings()
	}

	InitAudio();
}

void PauseMenuComponent::Reflect(Engine::Reflector& aReflector)
{
	aReflector.Reflect(mySettingsRef, "Settings");
}

void PauseMenuComponent::Execute(Engine::eEngineOrder aOrder)
{
	auto& input = GetEngine().GetInputManager();

	if (input.IsKeyDown(C::KeyCode::Escape) && myPlayer->GetStatus().isAlive)
	{
		if (myIsShowingSettings)
		{
			UIEventManager::CallEvent("SaveOptions", nullptr); //Smutsig lösning :^)
			Settings(nullptr);
		}
		else if (myIsShowingAreYouSure)
		{
			SureNo();
		}
		else
		{
			if(myPlayer->GetSpawnStateIsFinished())
				TogglePause();
		}
	}

	if (myIsPaused && !myIsShowingSettings && !myIsShowingAreYouSure)
	{
		Vec2f mousePos = GetEngine().GetViewport().myRelativeNormalizedMousePosition;

		auto item = myInventory->GetSelectedItem(mousePos);

		myItemDescription->ShowDesc(item);

		if (item)
		{
			myItemDescription->SetName(item->GetName());
			myItemDescription->SetDesc(item->GetDesc());
			myItemDescription->GetTransform().SetPosition({ mousePos.x, mousePos.y + 0.1f, 0.0f });

			if (myLastItem != item)
			{
				ItemHover* lastHover = myCurrentItemHover;
				ItemHover* newHover = lastHover == &myFirstItemHover ? &mySecondItemHover : &myFirstItemHover;
				
				lastHover->myIsShowingHover = false;

				newHover->myIsShowingHover = true;
				newHover->mySprite->GetTransform().SetPosition(myInventory->GetItemPosition(item));



				myCurrentItemHover = newHover;
			}
		}
		else
		{
			myCurrentItemHover->myIsShowingHover = false;
		}

		myLastItem = item;
	}


	myFirstItemHover.UpdateHover();
	mySecondItemHover.UpdateHover();

	UIEventManager::SetIsPaused(myIsPaused);
}

void PauseMenuComponent::SetPauseListener(std::function<void(bool)> aListener)
{
	myPauseListener = aListener;
}

void PauseMenuComponent::SetInventory(InventoryComponent* aInventory)
{
	myInventory = aInventory;
}

void PauseMenuComponent::TogglePause()
{
	if (myIsShowingAreYouSure)
		return;

	myIsPaused = !myIsPaused;

	if (myIsPaused)
	{
		myAudioComponent->PlayEvent("Pause");
		Time::PauseTimeScale = 0.0f;
		UpdatePlayerStats();

	}
	else if(!myIsPaused)
	{
		myAudioComponent->StopEvent("Pause");
		Time::PauseTimeScale = 1.0f;
	}

	myItemBox->SetActive(myIsPaused);
	myBackground->SetActive(myIsPaused);
	myItemDescription->ShowDesc(false);

	myResumeButton->SetActive(myIsPaused);
	mySettingsButton->SetActive(myIsPaused);
	myQuitButton->SetActive(myIsPaused);
	myStatsText->SetActive(myIsPaused);

	myFirstItemHover.mySprite->SetActive(myIsPaused);
	mySecondItemHover.mySprite->SetActive(myIsPaused);

	auto& input = GetEngine().GetInputManager();
	input.SetMouseWrapping(!myIsPaused);

	myCamera->SetIscontrollingCamera(!myIsPaused);

	if (myPauseListener)
		myPauseListener(myIsPaused);
}

void PauseMenuComponent::UpdatePlayerStats()
{
	HealthComponent* health = myPlayer->GetGameObject()->GetComponent<HealthComponent>();

	if (!myPlayer || !health)
		return;

	std::string stats = "";

	stats += "Health: ";
	stats += std::to_string(static_cast<int>(health->GetMaxHealth()));
	stats += "\n";

	stats += "Shield: ";
	stats += std::to_string(static_cast<int>(health->GetMaxShield()));
	stats += "\n";

	stats += "Damage: ";
	stats += std::to_string(static_cast<int>(myPlayer->GetStats().GetDamage()));
	stats += "\n";

	stats += "Crit Chance: ";
	stats += std::to_string(static_cast<int>(myPlayer->GetStats().GetCritChance() * 100.0f));
	stats += "%\n";

	stats += "Crit Damage: ";
	stats += std::to_string(static_cast<int>(myPlayer->GetStats().GetCritDamageMultiplier() * 100.0f));
	stats += "%\n";

	stats += "Attack Speed: ";
	stats += std::to_string(static_cast<int>(myPlayer->GetStats().GetAttackSpeed() * 100.0f));
	stats += "%\n";

	stats += "Dodge Chance: ";
	stats += std::to_string(static_cast<int>(health->GetDodgeChance() * 100.0f));
	stats += "%\n";

	stats += "Cooldown: ";
	stats += std::to_string(static_cast<int>(myPlayer->GetStats().GetCooldownMultiplier() * 100.0f));
	stats += "%\n";

	stats += "Walk Speed: ";
	stats += std::to_string(static_cast<int>((myPlayer->GetStats().GetMovementSpeed() / 700.0f) * 100.0f));
	stats += "%\n";

	stats += "Sprint Speed: ";
	stats += std::to_string(static_cast<int>(myPlayer->GetStats().GetSprintMultiplier() * 100.0f));
	stats += "%\n";

	stats += "Jump Force: ";
	stats += std::to_string(static_cast<int>((myPlayer->GetStats().GetJumpForce() / 600.0f) * 100.0f));
	stats += "%\n";

	stats += "Jump Amount: ";
	stats += std::to_string(static_cast<int>(myPlayer->GetStats().GetJumpAmount()));
	stats += "\n";

	myStatsText->SetText(stats);
}

const std::string PauseMenuComponent::GetGoodFloatText(float aFloat)
{
	std::string text = std::to_string(aFloat);

	for (size_t i = 0; i < text.size(); i++)
	{
		auto& letter = text[i];

		if (letter == '.')
		{
			for (size_t j = text.size() - 1; j > i + 2; j--)
			{
				text.pop_back();
			}

			letter = '-';
			break;
		}
	}

	return text;
}

void PauseMenuComponent::Resume(GameObject* aButton)
{
	TogglePause();
}

void PauseMenuComponent::Settings(GameObject* aButton)
{
	if (myIsShowingAreYouSure)
		return;

	myIsShowingSettings = !myIsShowingSettings;

	myItemBox->SetActive(!myIsShowingSettings);
	//myBackground->SetActive(!myIsShowingSettings);
	myResumeButton->SetActive(!myIsShowingSettings);
	mySettingsButton->SetActive(!myIsShowingSettings);
	myQuitButton->SetActive(!myIsShowingSettings);
	myStatsText->SetActive(!myIsShowingSettings);

	myFirstItemHover.mySprite->SetActive(!myIsShowingSettings);
	mySecondItemHover.mySprite->SetActive(!myIsShowingSettings);

	if (myPauseListener)
		myPauseListener(!myIsShowingSettings);

	if (myIsShowingSettings)
	{
		mySettings->GetTransform().SetPosition(Vec3f(0.0f, 0.0f, 0.0f));
	}
	else
	{
		mySettings->GetTransform().SetPosition(Vec3f(0.0f, 1.0f, 0.0f));
	}
}

void PauseMenuComponent::Quit(GameObject* aButton)
{
	if (myIsShowingAreYouSure)
		return;

	myAreYouSure->SetActive(true);
	myIsShowingAreYouSure = true;
}

void PauseMenuComponent::SureYes()
{
	UIEventManager::TryToFadeOut([&]()
	{
		GameManager::GetInstance()->SetShouldUpdateGameTime(true);
		myInventory->Clear();
		GameManager::GetInstance()->ChangeToMainMenu();
	});
}

void PauseMenuComponent::SureNo()
{
	myAreYouSure->SetActive(false);
	myIsShowingAreYouSure = false;
}

void PauseMenuComponent::InitAudio()
{
	myAudioComponent = myGameObject->AddComponent<Engine::AudioComponent>();
	myAudioComponent->AddEvent("Pause", "snapshot:/GAMESTATE/Paused");
}

void ItemHover::UpdateHover()
{
	myHoverFadeTimer += myIsShowingHover ? Time::DeltaTimeUnscaled : -Time::DeltaTimeUnscaled;
	myHoverFadeTimer = std::clamp(myHoverFadeTimer, 0.0f, myHoverFadeTime);

	float alpha = myHoverFadeTimer / myHoverFadeTime;
	mySprite->SetAlpha(alpha);
}