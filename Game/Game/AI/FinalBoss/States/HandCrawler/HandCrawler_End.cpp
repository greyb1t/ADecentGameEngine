#include "pch.h"
#include "HandCrawler_End.h"
#include "Engine/GameObject/GameObject.h"
#include "AI/PollingStation/PollingStationComponent.h"
#include "Engine/GameObject/Components/AnimatorComponent.h"
#include "AI/FinalBoss/Constants.h"
#include "AI/FinalBoss/RightProjectileHand.h"
#include "AI/FinalBoss/Behaviours/HandCrawler.h"

FB::HandCrawler_End::HandCrawler_End(HandCrawler& aParent)
	: myParent(aParent)
{
	aParent.GetBossHand().GetAnimator().GetController().AddStateOnExitCallback(
		"Base", "Crawl End", [this]() { myIsFinished = true; });
}

void FB::HandCrawler_End::Update()
{
}

bool FB::HandCrawler_End::IsFinished()
{
	return myIsFinished;
}

void FB::HandCrawler_End::OnEnter()
{
	myParent.GetBossHand().GetAnimator().GetController().SetInt(Constants::CrawlEyeStateIntName, 2);

	// auto polling = myParent.GetRightHand().GetGameObject()->GetSingletonComponent<PollingStationComponent>();
	// const auto playerPos = polling->GetPlayerPos();

	myIsFinished = false;
}

void FB::HandCrawler_End::OnExit()
{
}
