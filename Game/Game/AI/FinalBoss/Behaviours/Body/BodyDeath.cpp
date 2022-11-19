#include "pch.h"
#include "BodyDeath.h"
#include "AI/FinalBoss/Constants.h"
#include "Engine/GameObject/Components/AnimatorComponent.h"
#include "Engine/GameObject/Components/AudioComponent.h"
#include "AI/FinalBoss/FinalBossBody.h"
#include "AI/FinalBoss/FinalBoss.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/GameObject/Components/SpriteComponent.h"
#include "Engine/GameObject/Components/VideoComponent.h"
#include "Engine/GameObject/Prefab/GameObjectPrefab.h"
#include "AI/PollingStation/PollingStationComponent.h"
#include "Components/HealthComponent.h"
#include "Engine/GameObject/Components/MusicManager.h"

FB::BodyDeath::BodyDeath(FinalBossBody& aBody)
	: myBody(aBody)
{
	aBody.GetAnimator().GetController().AddStateOnExitCallback("Base", "Death", [this]() { OnFinished(); });
	// aBody.GetAnimator().GetController().AddEventCallback("StartFading", [this]() { myStartFading = true; });
	aBody.GetAnimator().GetController().AddEventCallback("StartFalling",
		[this]()
		{
			myBody.GetBodyAudio()->PlayEvent("Falling");
		});
}

void FB::BodyDeath::Update()
{
	Main::SetCanOpenChestShield(true);

	// to not make it in the ground
	const auto& posLocal = myBody.GetModelGameObject()->GetTransform().GetPositionLocal();
	const Vec3f targetPos = Vec3f(0.f, 200.f, 0.f);
	const auto resultPos = Math::Lerp(posLocal, targetPos, Time::DeltaTime * 1.f);
	myBody.GetModelGameObject()->GetTransform().SetPositionLocal(resultPos);

	myStartFadingTimer.Tick(Time::DeltaTime);

	if (myStartFadingTimer.JustFinished())
	{
		// Just in case the event does not run
		myStartFading = true;
	}

	if (myStartFading)
	{
		mySpriteAlpha += 0.2f * Time::DeltaTime;
		mySpriteAlpha = Math::Clamp(mySpriteAlpha, 0.f, 1.f);

		if (myWhiteSprite)
		{
			myWhiteSprite->SetColor(Vec4f(1.f, 1.f, 1.f, mySpriteAlpha));
		}

		if (mySpriteAlpha >= 1.f && !myHasStartedChangingToMainMenu)
		{
			myHasStartedChangingToMainMenu = true;

			if (myWhiteSpriteGameObject)
			{
				myWhiteSpriteGameObject->SetActive(false);
				myWhiteSpriteGameObject->Destroy();
			}

			GameManager::GetInstance()->SetShouldUpdateGameTime(false);
			// Time::TimeScale = 0.0f;

			PlayCreditsVideo();
		}
	}

	if (myEndCredits && myEndCredits->IsActive())
	{
		auto& input = GetEngine().GetInputManager();

		if (input.IsKeyDown(C::KeyCode::Escape))
			myEndCredits->Finish();
	}
}

bool FB::BodyDeath::IsFinished() const
{
	return false;
}

void FB::BodyDeath::OnEnter()
{
	myBody.GetAnimator().GetController().Trigger(Constants::DeathTriggerName);
	myBody.GetBodyAudio()->PlayEvent("Death");

	if (auto mm = myBody.GetGameObject()->GetSingletonComponent<Engine::MusicManager>())
	{
		mm->Stop();
	}

	myStartFading = false;
	myHasStartedChangingToMainMenu = false;

	const auto& whiteSpritePrefab = myBody.GetFinalBoss().GetWhiteSpritePrefab();

	myWhiteSprite = nullptr;

	if (whiteSpritePrefab && whiteSpritePrefab->IsValid())
	{
		myWhiteSpriteGameObject = &whiteSpritePrefab->Get().Instantiate(*myBody.GetFinalBoss().GetGameObject()->GetScene());

		myWhiteSprite = myWhiteSpriteGameObject->GetComponent<Engine::SpriteComponent>();
	}

	myStartFadingTimer = TickTimer::FromSeconds(myBody.GetFinalBoss().GetTimeUntilFadeToCredits());

	MakePlayerInvincible();

	KillSurroundingEnemies();

	/*
	auto whiteSpriteGameObject = myBody.GetFinalBoss().GetGameObject()->GetScene()->AddGameObject<GameObject>();
	whiteSpriteGameObject->GetTransform().SetPosition(Vec3f(0.5f, 0.5f, 10000.f));
	whiteSpriteGameObject->GetTransform().SetScale(1000.f);
	myWhiteSprite = whiteSpriteGameObject->AddComponent<Engine::SpriteComponent>();
	myWhiteSprite->SetColor(Vec4f(1.f, 1.f, 1.f, 0.f));
	mySpriteAlpha = 0.f;
	*/
}

void FB::BodyDeath::OnExit()
{
}

void FB::BodyDeath::Reflect(Engine::Reflector& aReflector)
{
}

void FB::BodyDeath::OnFinished()
{
	//GameManager::GetInstance()->SetShouldUpdateGameTime(false);

	//Time::TimeScale = 0.0f;

	//// Start video of credits
	//LOG_INFO(LogType::Game) << "START CREDITS VIDEO";

	//// Just in case the event does not run
	//myStartFading = true;
}

void FB::BodyDeath::PlayCreditsVideo()
{
	const auto& creditsVideo = myBody.GetFinalBoss().GetCreditsVideoPrefab();

	if (creditsVideo && creditsVideo->IsValid())
	{
		auto& g = creditsVideo->Get().Instantiate(*myBody.GetFinalBoss().GetGameObject()->GetScene());
		g.SetActive(true);

		if (auto video = g.GetComponent<Engine::VideoComponent>())
		{
			myEndCredits = video;

			// video->Play(false);
			video->SetCallback(
				[]()
				{
					Main::SaveProgress();
					GameManager::GetInstance()->ChangeToWinScreen();
					LOG_INFO(LogType::Game) << "changed to win screen";

					//GameManager::GetInstance()->ChangeToMainMenu();
					//LOG_INFO(LogType::Game) << "changed to main menu";
				});
		}
	}
	else
	{
		LOG_ERROR(LogType::Game) << "credits video prefab missing";
	}
}

void FB::BodyDeath::MakePlayerInvincible()
{
	auto polling = myBody.GetGameObject()->GetSingletonComponent<PollingStationComponent>();

	if (GameObject* playerObj = polling->GetPlayer())
	{
		if (auto playerHealth = playerObj->GetComponent<HealthComponent>())
		{
			playerHealth->SetIsInvincible(true);
		}
		else
		{
			LOG_ERROR(LogType::Game) << "Player missing HealthComponent";
		}
	}
}

void FB::BodyDeath::KillSurroundingEnemies()
{
	std::vector<OverlapHit> hits;

	const bool hitSomething = myBody.GetGameObject()->GetScene()->SphereCastAll(
		myBody.GetFinalBoss().GetTransform().GetPosition(),
		20000.f,
		eLayer::ENEMY_DAMAGEABLE | eLayer::ENEMY,
		hits,
		true);

	if (hitSomething)
	{
		for (const auto& hit : hits)
		{
			if (hit.GameObject)
			{
				if (hit.GameObject->GetTag() == eTag::ENEMY ||
					hit.GameObject->GetTag() == eTag::ENEMYPROJECTILE)
				{
					if (auto health = hit.GameObject->GetComponent<HealthComponent>())
					{
						health->ApplyDamage(1000000000);
					}
				}
			}
		}
	}
}
