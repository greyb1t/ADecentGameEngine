#include "pch.h"
#include "MenuHandler.h"

#include "MenuComponent.h"
#include "Engine/GameObject/Components/SpriteComponent.h"
#include "Engine/GameObject/Components/VideoComponent.h"

void MenuHandler::Awake()
{
	if (auto title = myTitleImageRef.Get())
	{
		if (myTitleImage = title->GetComponent<Engine::SpriteComponent>())
		{
			myTitleImage->SetAlpha(0.0f);
		}
	}

	if (auto video = myBackgroundVideoRef.Get())
	{
		myBackgroundVideo = video->GetComponent<Engine::VideoComponent>();
	}

	if (auto menu = myMainMenuRef.Get())
	{
		if (auto component = menu->GetComponent<MenuComponent>())
		{
			component->myMenuHandler = this;
		}

		const int childCount = menu->GetTransform().GetChildren().size() - 1;
		for (int i = 0; i < childCount; i++)
		{
			auto& parentTransform = menu->GetTransform();
			if (auto child = parentTransform.GetChildren()[i])
			{
				MenuAnimation anim;
				anim.myMenuObj = child->GetGameObject();
				anim.myStartPos = child->GetPosition();
				anim.myStartPos.x = anim.myStartPos.x * -1.0f * 3.0f;
				anim.myEndPos = child->GetPosition();
				anim.myStartTime = (static_cast<float>(i) / static_cast<float>(childCount)) * myAnimationDuration;
				anim.myProgress = anim.myDuration;
				myAnims.push_back(anim);

				child->SetPosition(anim.myStartPos);
			}
		}
		myAnimationProgress = 0.0f;
		myTitleImageFadeProgress = 0.0f;
		myAnimationStartDelayProgress = 0.0f;
	}

	if (auto menu = myPlayRef.Get())
	{
		if (auto component = menu->GetComponent<MenuComponent>())
		{
			component->myMenuHandler = this;
		}
	}

	if (auto menu = myLevelSelectRef.Get())
	{
		if (auto component = menu->GetComponent<MenuComponent>())
		{
			component->myMenuHandler = this;
		}
	}

	if (auto menu = myHowToPlayRef.Get())
	{
		if (auto component = menu->GetComponent<MenuComponent>())
		{
			component->myMenuHandler = this;
		}
	}

	if (auto menu = myOptionsRef.Get())
	{
		if (auto component = menu->GetComponent<MenuComponent>())
		{
			component->myMenuHandler = this;
		}
	}

	if (auto menu = myCreditsRef.Get())
	{
		if (auto component = menu->GetComponent<MenuComponent>())
		{
			component->myMenuHandler = this;
		}
	}

	if (auto menu = myLoadingScreenRef.Get())
	{
		if (auto component = menu->GetComponent<MenuComponent>())
		{
			component->myMenuHandler = this;
		}
	}
}

void MenuHandler::Start()
{
	UI::AddEvent("BackToMainMenu", [this](GameObject* aButtonComponent) { EnableMainMenu(); });

	EnableMainMenu();

	if (GameManager::GetInstance()->GetGameSettings().myGameHasStartedFlag)
	{
		SkipToVideoLoop();
		myMenuState = MenuState::Animating;
	}
}

void MenuHandler::Reflect(Engine::Reflector& aReflector)
{
	aReflector.Reflect(myTitleImageRef, "Title Image");
	aReflector.Reflect(myBackgroundVideoRef, "Background Video");
	aReflector.Reflect(myMainMenuRef, "Main Menu");
	aReflector.Reflect(myPlayRef, "Play");
	aReflector.Reflect(myLevelSelectRef, "Level Select");
	aReflector.Reflect(myHowToPlayRef, "How To Play");
	aReflector.Reflect(myOptionsRef, "Options");
	aReflector.Reflect(myCreditsRef, "Credits");
	aReflector.Reflect(myLoadingScreenRef, "Loading Screen");
}

void MenuHandler::Execute(Engine::eEngineOrder aOrder)
{
	switch (myMenuState)
	{
	case MenuHandler::MenuState::NotStarted: 
		NotStarted(); 
		break;
	case MenuHandler::MenuState::Animating: 
		Animating(); 
		break;
	case MenuHandler::MenuState::Finished: 
		Finished(); 
		break;
	default:
		break;
	}
}

void MenuHandler::AnimateIntro()
{
	if (myMenuState != MenuState::Finished)
	{
		myMenuState = MenuState::Animating;
	}
	else 
	{
		EnableMainMenu();
	}
}

void MenuHandler::EnableMainMenu()
{
	DisableAll();
	if (auto menu = myMainMenuRef.Get())
		menu->SetActive(true);


	GameManager::GetInstance()->myStartedOnBossLevel = false;
}

void MenuHandler::EnablePlay()
{
	DisableAll();
	if (auto menu = myPlayRef.Get())
		menu->SetActive(true);
}

void MenuHandler::EnableLevelSelect()
{
	DisableAll();
	if (auto menu = myLevelSelectRef.Get())
		menu->SetActive(true);
}

void MenuHandler::EnableHowToPlay()
{
	DisableAll();
	if (auto menu = myHowToPlayRef.Get())
		menu->SetActive(true);
}

void MenuHandler::EnableOptions()
{
	DisableAll();
	if (auto menu = myOptionsRef.Get())
		menu->SetActive(true);
}

void MenuHandler::EnableCredits()
{
	DisableAll();
	if (auto menu = myCreditsRef.Get())
		menu->SetActive(true);
}

void MenuHandler::EnableLoadingScreen()
{
	DisableAll();
	if (auto menu = myLoadingScreenRef.Get())
		menu->SetActive(true);
}

void MenuHandler::SkipToVideoLoop()
{
	if (!myBackgroundVideo)
		return;

	const float time = myBackgroundVideo->myLoopStartTime;
	myBackgroundVideo->SeekToSeconds(time);
	myAnimationStartDelayProgress = myAnimationStartDelayDuration;
}

void MenuHandler::SkipMenuAnimation()
{
	myTitleImageFadeProgress = myTitleImageFadeDuration;

	for (int i = 0; i < myAnims.size(); i++)
	{
		auto& anim = myAnims[i];

		if (auto obj = anim.myMenuObj)
		{
			obj->GetTransform().SetPosition(anim.myEndPos);
		}
	}
	myAnims.clear();

	myTitleImage->SetAlpha(1.f);

	myMenuState = MenuState::Finished;

	GameManager::GetInstance()->GetGameSettings().myGameHasStartedFlag = true;
}

void MenuHandler::DisableAll()
{
	if (auto menu = myMainMenuRef.Get())
		menu->SetActive(false);

	if (auto menu = myPlayRef.Get())
		menu->SetActive(false);

	if (auto menu = myLevelSelectRef.Get())
		menu->SetActive(false);
	
	if (auto menu = myHowToPlayRef.Get())
		menu->SetActive(false);

	if (auto menu = myOptionsRef.Get())
		menu->SetActive(false);

	if (auto menu = myCreditsRef.Get())
		menu->SetActive(false);

	if (auto menu = myLoadingScreenRef.Get())
		menu->SetActive(false);
}

void MenuHandler::SetSelectedLevel(const std::string& aLevelPath)
{
	mySelectedLevel = aLevelPath;
}

void MenuHandler::ClearSelectedLevel()
{
	SetSelectedLevel("");
}

const std::string& MenuHandler::GetSelectedLevel() const
{
	return mySelectedLevel;
}

void MenuHandler::NotStarted()
{
}

void MenuHandler::Animating()
{
	auto& input = GetEngine().GetInputManager();
	
	myAnimationStartDelayProgress += Time::DeltaTime;
	if (myAnimationStartDelayProgress < myAnimationStartDelayDuration)
	{
		if (input.IsKeyDown(C::KeyCode::Escape))
		{
			SkipToVideoLoop();
		}
		return;
	}

	myTitleImageFadeProgress += Time::DeltaTime;
	if (myTitleImageFadeProgress >= myTitleImageFadeDuration)
		myTitleImageFadeProgress = myTitleImageFadeDuration;

	const float ratio = myTitleImageFadeProgress / myTitleImageFadeDuration;
	myTitleImage->SetAlpha(ratio);

	if (myTitleImageFadeProgress < myTitleImageFadeDuration)
	{
		if (input.IsKeyDown(C::KeyCode::Escape))
		{
			SkipMenuAnimation();
		}
		return;
	}

	if (input.IsKeyDown(C::KeyCode::Escape))
	{
		SkipMenuAnimation();
		return;
	}

	myAnimationProgress += Time::DeltaTime;

	const int count = myAnims.size() - 1;
	for (int i = count; i >= 0; i--)
	{
		bool remove = false;
		auto& anim = myAnims[i];

		if (myAnimationProgress >= anim.myStartTime)
		{
			anim.myProgress -= Time::DeltaTime;
			if (anim.myProgress <= 0.0f)
			{
				anim.myProgress = 0.f;
				remove = true;
			}
		}

		const float ratio = 1.0f - (anim.myProgress / anim.myDuration);
		Vec3f pos = CU::Lerp(anim.myStartPos, anim.myEndPos, ratio);

		if (auto obj = anim.myMenuObj)
		{
			obj->GetTransform().SetPosition(pos);
		}

		if (remove)
		{
			myAnims.erase(myAnims.begin() + i);
		}
	}

	if (myAnims.empty())
	{
		GameManager::GetInstance()->GetGameSettings().myGameHasStartedFlag = true;
		myMenuState = MenuState::Finished;
	}
}

void MenuHandler::Finished()
{
	auto& input = GetEngine().GetInputManager();
	if (input.IsKeyDown(CU::KeyCode::Escape))
	{
		if (myOptionsRef.Get()->IsActive())
		{
			auto gm = GameManager::GetInstance();
			gm->GetGameSettings().Save();
		}

		//EnableMainMenu();
	}
}
