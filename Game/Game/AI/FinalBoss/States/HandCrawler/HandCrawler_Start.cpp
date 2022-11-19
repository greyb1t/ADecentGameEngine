#include "pch.h"
#include "HandCrawler_Start.h"
#include "Engine/GameObject/GameObject.h"
#include "AI/PollingStation/PollingStationComponent.h"
#include "Engine/GameObject/Components/AnimatorComponent.h"
#include "AI/FinalBoss/Constants.h"
#include "AI/FinalBoss/Behaviours/HandCrawler.h"
#include "Engine/AnimationCurve/Curve.h"
#include "Engine/ResourceManagement/Resources/AnimationCurveResource.h"
#include "AI/FinalBoss/FinalBossHand.h"

FB::HandCrawler_Start::HandCrawler_Start(HandCrawler& aParent)
	: myCrawler(aParent)
{
	myCrawler.GetBossHand().GetAnimator().GetController().AddStateOnExitCallback("Base", "Crawl Start",
		[this]()
		{
			myIsFinished = true;
		});
}

void FB::HandCrawler_Start::Update()
{
	// myTimer.Tick(Time::DeltaTime);
	myTimeElapsed += Time::DeltaTime;

	float t = 1.f;

	if (myDecentSpeedCurve)
	{
		t = myDecentSpeedCurve->Evaluate(myTimeElapsed);
	}
	else
	{
		LOG_ERROR(LogType::Game) << "Hand crawler is missing decend speed curve";
	}

	if (t >= 1.f && !myHasTriggeredAnimation)
	{
		myHasTriggeredAnimation = true;

		myCrawler.GetBossHand().GetAnimator().GetController().Trigger(Constants::CrawlEyeTriggerName);
		myCrawler.GetBossHand().GetAnimator().GetController().SetInt(Constants::CrawlEyeStateIntName, 0);
	}

	const auto pos = mySpline.Evaluate(t);

	myCrawler.GetBossHand().GetGameObject()->GetTransform().SetPosition(pos);
}

bool FB::HandCrawler_Start::IsFinished()
{
	if (myDecentSpeedCurve)
	{
		return myTimeElapsed >= myDecentSpeedCurve->GetMaximumTime() && myIsFinished;
	}

	return myIsFinished;
}

void FB::HandCrawler_Start::OnEnter()
{
	myIsFinished = false;
	myHasTriggeredAnimation = false;

	if (myDecentSpeedCurve)
	{
		myTimeElapsed = myDecentSpeedCurve->GetMinimumTime();
	}

	// Must do this here because cannot call virtual functions from constructor
	if (!myDecentSpeedCurve)
	{
		if (myCrawler.Desc().myDecentSpeedCurve &&
			myCrawler.Desc().myDecentSpeedCurve->IsValid())
		{
			myDecentSpeedCurve = &myCrawler.Desc().myDecentSpeedCurve->Get();
		}
		else
		{
			LOG_ERROR(LogType::Game) << "Hand crawler is missing decend speed curve";
		}
	}

	RayCastHit hit;

	Vec3f groundPos;

	if (myCrawler.GetBossHand().GetGameObject()->GetScene()->RayCast(
		myCrawler.GetBossHand().GetGameObject()->GetTransform().GetPosition(),
		Vec3f(0.f, -1.f, 0.f),
		Constants::RaycastDistance,
		/*eLayer::NAVMESH*/eLayer::GROUND | eLayer::DEFAULT,
		hit))
	{
		groundPos = hit.Position;
	}
	else
	{
		LOG_ERROR(LogType::Game) << "Finalboss raycast navmesh failed";
	}

	mySpline = {};
	mySpline.AddPoint(myCrawler.GetBossHand().GetGameObject()->GetTransform().GetPosition());
	mySpline.AddPoint(groundPos + Vec3f(0.f, 1.f, 0.f) * myCrawler.Desc().myOffsetAboveGround);
}

void FB::HandCrawler_Start::OnExit()
{
}
