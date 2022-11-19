#include "pch.h"
#include "WinScreen.h"

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

WinScreen::WinScreen(GameObject* aGameObject) : Component(aGameObject)
{

}

WinScreen::~WinScreen()
{

}

void WinScreen::Start()
{
	GameObject* playerObj = Main::GetPlayer();
	myPlayer = playerObj->GetComponent<Player>();

	playerObj->GetTransform().GetParent()->SetPosition(Vec3f(1.0f, 10.0f, 0.0f));

	myCamera = playerObj->GetComponent<CameraController>();
	myInventory = playerObj->GetComponent<InventoryComponent>();

	myItemDescription = GetTransform().GetChildByGameObjectName("PauseItemDesc")->GetComponent<HUDItemDescription>();
	myStatsText = GetTransform().GetChildByGameObjectName("Stats")->GetComponent<Engine::TextComponent>();
	myTimeText = GetTransform().GetChildByGameObjectName("Time")->GetTransform().GetChildByGameObjectName("TimeText")->GetComponent<Engine::TextComponent>();
	myTimeTextMil = GetTransform().GetChildByGameObjectName("Time")->GetTransform().GetChildByGameObjectName("TimeTextMil")->GetComponent<Engine::TextComponent>();

	UIEventManager::AddEvent("WinMainMenu", [&](GameObject* aButton) { Quit(aButton); });

	Time::PauseTimeScale = 1.0f;

	auto& gameDifficulty = GetTransform().GetChildByGameObjectName("GameDifficulty")->GetTransform();

	myDifficultyText = gameDifficulty.GetChildByGameObjectName("Text")->GetComponent<Engine::TextComponent>();

	auto& icons = gameDifficulty.GetChildByGameObjectName("Icons")->GetTransform();
	myEye1 = icons.GetChildByGameObjectName("1")->GetComponent<Engine::SpriteComponent>();
	myEye2 = icons.GetChildByGameObjectName("2")->GetComponent<Engine::SpriteComponent>();
	myEye3 = icons.GetChildByGameObjectName("3")->GetComponent<Engine::SpriteComponent>();
	myEye4 = icons.GetChildByGameObjectName("4")->GetComponent<Engine::SpriteComponent>();
	myEye5 = icons.GetChildByGameObjectName("5")->GetComponent<Engine::SpriteComponent>();
	myEye6 = icons.GetChildByGameObjectName("6")->GetComponent<Engine::SpriteComponent>();
	myEye7 = icons.GetChildByGameObjectName("7")->GetComponent<Engine::SpriteComponent>();

	myStarPopup = GetTransform().GetChildByGameObjectName("NewDifficulty")->GetComponent<Engine::SpriteComponent>();
	myStarPopup->SetActive(false); 

	InitAudio();
}

void WinScreen::Reflect(Engine::Reflector& aReflector)
{
	
}

void WinScreen::Execute(Engine::eEngineOrder aOrder)
{
	if (myIsFirstFrame)
	{
		myIsFirstFrame = false;
		UpdatePlayerStats();
		SetTimeText();
		myInventory->ShowItems(true);
		SetDifficulty();
		SaveToLeaderboard();
	}

	myCamera->SetIscontrollingCamera(false);

	{
		Vec2f mousePos = GetEngine().GetViewport().myRelativeNormalizedMousePosition;

		auto item = myInventory->GetSelectedItem(mousePos);

		myItemDescription->ShowDesc(item);

		if (item)
		{
			myItemDescription->SetName(item->GetName());
			myItemDescription->SetDesc(item->GetDesc());
			myItemDescription->GetTransform().SetPosition({ mousePos.x, mousePos.y + 0.1f, 0.0f });
		}
	}
}

void WinScreen::UpdatePlayerStats()
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

const std::string WinScreen::GetGoodFloatText(float aFloat)
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

			letter = ':';
			break;
		}
	}

	return text;
}

void WinScreen::Quit(GameObject* aButton)
{
	UIEventManager::TryToFadeOut([&]()
	{
		GameManager::GetInstance()->SetShouldUpdateGameTime(true);
		GameManager::GetInstance()->Reset();
		myInventory->Clear();
		GameManager::GetInstance()->ChangeToMainMenu();
		LOG_INFO(LogType::Simon) << "LOADING MANI MANUE";
	});
}

void WinScreen::InitAudio()
{
	myAudioComponent = myGameObject->AddComponent<Engine::AudioComponent>();
	myAudioComponent->AddEvent("Pause", "snapshot:/GAMESTATE/Paused");
}

void WinScreen::SetTimeText()
{
	myLastTime = GameManager::GetInstance()->GetTotalGameTime();

	double dSeconds = 0.0f;
	double dMilsecs = std::modf(static_cast<double>(myLastTime), &dSeconds);

	int seconds = static_cast<int>(dSeconds);
	int milsecs = static_cast<int>(dMilsecs * 100.0);

	int minutes = 0;

	if (seconds > 60)
	{
		minutes = seconds / 60;
		seconds %= 60;
	}

	std::string time = "";
	std::string timeMil = "";

	if (minutes < 10)
		time += "0";
	time += std::to_string(minutes);
	time += ":";

	if (seconds < 10)
		time += "0";
	time += std::to_string(seconds);

	timeMil += "-";
	if (milsecs < 10)
		timeMil += "0";
	timeMil += std::to_string(milsecs);


	myTimeText->SetText(time);
	myTimeTextMil->SetText(timeMil);
}

void WinScreen::SetDifficulty()
{
	float difficulty = GameManager::GetInstance()->GetGameSettings().myDifficultyMultiplier;
	std::string difficultyText = "";

	if(difficulty == 0.1f)
	{
		difficultyText += "Educator's Mode";
		myEye1->SetActive(true);
	}
	else if (difficulty == 0.75f)
	{
		difficultyText += "Beginner";
		myEye2->SetActive(true);
	}
	else if (difficulty == 1.0f)
	{
		difficultyText += "Normal";
		myEye3->SetActive(true);
	}
	else if (difficulty == 1.5f)
	{
		difficultyText += "Hard";
		myEye4->SetActive(true);
	}
	else if (difficulty == 2.0f)
	{
		difficultyText += "Nightmare";
		myEye5->SetActive(true);
	}
	else if (difficulty == 4.0f)
	{
		difficultyText += "Impossible";
		myEye6->SetActive(true);

		if (GameManager::GetInstance()->GetGameSettings().myHasUnlockedStarMode == false) 
		{
			GameManager::GetInstance()->GetGameSettings().myHasUnlockedStarMode = true;
			GameManager::GetInstance()->GetGameSettings().Save();

			myStarPopup->SetActive(true);
		}
	}
	else if (difficulty == 8.0f)
	{
		difficultyText += "Star Mode";
		myEye7->SetActive(true);
	}

	myDifficultyText->SetText(difficultyText);
}

void WinScreen::SaveToLeaderboard()
{
	auto gm = GameManager::GetInstance();
	
	if (gm->myStartedOnBossLevel)
	{
		gm->myStartedOnBossLevel = false;
		return;
	}

	float difficulty = gm->GetGameSettings().myDifficultyMultiplier;
	
	auto& lb = gm->GetLeaderboard();

	lb.AddScore(myLastTime, difficulty);
	lb.Save();
}
