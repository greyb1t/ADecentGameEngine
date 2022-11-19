#include "pch.h"
#include "HandCrawler.h"
#include "AI/FinalBoss/States/HandCrawler/HandCrawler_Start.h"
#include "AI/FinalBoss/States/HandCrawler/HandCrawler_Loop.h"
#include "AI/FinalBoss/States/HandCrawler/HandCrawler_End.h"
#include "AI/FinalBoss/FinalBossHand.h"

FB::HandCrawler::HandCrawler(FinalBossHand& aBossHand)
	: myBossHand(aBossHand)
{
	myStates[HandCrawlerStates::Start] = MakeOwned<HandCrawler_Start>(*this);
	myStates[HandCrawlerStates::Loop] = MakeOwned<HandCrawler_Loop>(*this);
	myStates[HandCrawlerStates::End] = MakeOwned<HandCrawler_End>(*this);

	myStateSequencer.AddState(myStates[HandCrawlerStates::Start].get());
	myStateSequencer.AddState(myStates[HandCrawlerStates::Loop].get());
	myStateSequencer.AddState(myStates[HandCrawlerStates::End].get());
}

void FB::HandCrawler::Update()
{
	myStateSequencer.Update();
}

bool FB::HandCrawler::IsFinished() const
{
	return myStateSequencer.IsFinished();
}

void FB::HandCrawler::OnEnter()
{
	myStateSequencer.Init();
}

void FB::HandCrawler::OnExit()
{
	myStateSequencer.OnExit();
}

void FB::HandCrawler::Reflect(Engine::Reflector& aReflector)
{
	HandCrawlerStates myStateType = HandCrawlerStates::_from_integral(myStateSequencer.GetActiveStateIndex());
	aReflector.ReflectEnum(myStateType, "State");

	if (myStateSequencer.GetActiveState())
	{
		myStateSequencer.GetActiveState()->Reflect(aReflector);
	}
}

FB::FinalBossHand& FB::HandCrawler::GetBossHand()
{
	return myBossHand;
}

const FB::HandCrawlerDesc& FB::HandCrawler::Desc() const
{
	return myBossHand.GetHandCrawlerDesc();
}
