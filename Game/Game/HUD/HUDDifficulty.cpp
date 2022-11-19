#include "pch.h"
#include "HUDDifficulty.h"

#include "Engine/Engine.h"

#include "Engine/GameObject/Components/SpriteComponent.h"
#include "Engine/GameObject/Components/TextComponent.h"

#include "Engine\Reflection\Reflector.h"
#include "Engine\GameObject\GameObject.h"

#include "Game/GameManager/GameManager.h"

HUDDifficulty::HUDDifficulty(GameObject* aGameObject) : Component(aGameObject)
{
}

HUDDifficulty::~HUDDifficulty()
{
}

void HUDDifficulty::Reflect(Engine::Reflector& aReflector)
{
}

void HUDDifficulty::Start()
{
	myTimeText = GetTransform().GetChildByGameObjectName("TimeText")->GetComponent<Engine::TextComponent>();
	myTimeTextMil = GetTransform().GetChildByGameObjectName("TimeTextMil")->GetComponent<Engine::TextComponent>();
	myStageText = GetTransform().GetChildByGameObjectName("Stage Number")->GetComponent<Engine::TextComponent>();

	auto& icons = GetTransform().GetChildByGameObjectName("Icons")->GetTransform();
	myEye1 = icons.GetChildByGameObjectName("1")->GetComponent<Engine::SpriteComponent>();
	myEye2 = icons.GetChildByGameObjectName("2")->GetComponent<Engine::SpriteComponent>();
	myEye3 = icons.GetChildByGameObjectName("3")->GetComponent<Engine::SpriteComponent>();
	myEye4 = icons.GetChildByGameObjectName("4")->GetComponent<Engine::SpriteComponent>();
	myEye5 = icons.GetChildByGameObjectName("5")->GetComponent<Engine::SpriteComponent>();
	myEye6 = icons.GetChildByGameObjectName("6")->GetComponent<Engine::SpriteComponent>();
	myEye7 = icons.GetChildByGameObjectName("7")->GetComponent<Engine::SpriteComponent>();


	myStageText->SetText(std::to_string(GameManager::GetInstance()->GetLevelChangeCount()));

	GameObject* child = GetTransform().GetChildByGameObjectName("Difficulty");
	if(!child)
	{
		LOG_ERROR(LogType::Game) << "No child named Difficulty found.";
		return;
	}

	Engine::SpriteComponent* sprite = child->GetComponent<Engine::SpriteComponent>();
	if(!sprite)
	{
		LOG_ERROR(LogType::Game) << "No SpriteComponent found.";
		return;
	}

	myMaterialInstance = &sprite->GetMaterialInstance();

	UpdateDifficultyIcon();
}

void HUDDifficulty::Execute(Engine::eEngineOrder aOrder)
{
	myMaterialInstance->SetFloat4("myTime1", { GameManager::GetInstance()->GetDifficultyNormalized(), 0.f, 0.f, 1.f });

	UpdateTime();
}

void HUDDifficulty::IterateStageNumber()
{
	myStageNumber++;

	myStageText->SetText(std::to_string(myStageNumber));
}

void HUDDifficulty::UpdateTime()
{
	double dSeconds = 0.0f;
	double dMilsecs = std::modf(static_cast<double>(GameManager::GetInstance()->GetTotalGameTime()), &dSeconds);

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

void HUDDifficulty::UpdateDifficultyIcon()
{
	//auto difficulty = GameManager::GetInstance()->GetGameDifficulty();

	//if (difficulty != myCurrentDifficulty)
	//{
	//	myCurrentDifficulty = difficulty;

	//	myEye1->SetActive(false);
	//	myEye2->SetActive(false);
	//	myEye3->SetActive(false);
	//	myEye4->SetActive(false);
	//	myEye5->SetActive(false);

	//	switch (myCurrentDifficulty)
	//	{
	//	case GameDifficulty::Beginner:
	//		myEye1->SetActive(true);
	//		break;

	//	case GameDifficulty::Easy:
	//		myEye2->SetActive(true);
	//		break;

	//	case GameDifficulty::Medium:
	//		myEye3->SetActive(true);
	//		break;

	//	case GameDifficulty::Hard:
	//		myEye4->SetActive(true);
	//		break;

	//	case GameDifficulty::Extreme:
	//		myEye5->SetActive(true);
	//		break;

	//	case GameDifficulty::Haha:
	//		myEye6->SetActive(true);
	//		break;

	//	default:
	//		break;
	//	}
	//}

	myEye1->SetActive(false);
	myEye2->SetActive(false);
	myEye3->SetActive(false);
	myEye4->SetActive(false);
	myEye5->SetActive(false);
	myEye6->SetActive(false);
	myEye7->SetActive(false);

	float difficulty = GameManager::GetInstance()->GetGameSettings().myDifficultyMultiplier;

	if (difficulty == 0.1f)
	{
		myEye1->SetActive(true);
	}
	else if (difficulty == 0.75f)
	{
		myEye2->SetActive(true);
	}
	else if (difficulty == 1.0f)
	{
		myEye3->SetActive(true);
	}
	else if (difficulty == 1.5f)
	{
		myEye4->SetActive(true);
	}
	else if (difficulty == 2.0f)
	{
		myEye5->SetActive(true);
	}
	else if (difficulty == 4.0f)
	{
		myEye6->SetActive(true);
	}
	else if (difficulty == 4.0f)
	{
		myEye7->SetActive(true);
	}
}
