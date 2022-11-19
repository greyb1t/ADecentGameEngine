#include "pch.h"
#include "LeftHand_HandCrawler.h"
#include "Engine/GameObject/Components/AnimatorComponent.h"
#include "AI/FinalBoss/Constants.h"
#include "Engine/GameObject/Prefab/GameObjectPrefab.h"
#include "Engine/GameObject/GameObject.h"
#include "AI/FinalBoss/LeftPunchHand.h"

FB::LeftHand_HandCrawler::LeftHand_HandCrawler(
	LeftPunchHand& aLeftHand)
	: HandCrawler(aLeftHand),
	myLeftHand(aLeftHand)
{
}

FB::LeftPunchHand& FB::LeftHand_HandCrawler::GetLeftHand()
{
	return myLeftHand;
}

