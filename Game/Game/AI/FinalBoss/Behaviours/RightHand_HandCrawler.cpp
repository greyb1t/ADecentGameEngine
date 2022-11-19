#include "pch.h"
#include "RightHand_HandCrawler.h"
#include "Engine/GameObject/Components/AnimatorComponent.h"
#include "AI/FinalBoss/RightProjectileHand.h"
#include "AI/FinalBoss/Constants.h"
#include "Engine/GameObject/Prefab/GameObjectPrefab.h"
#include "Engine/GameObject/GameObject.h"

FB::RightHand_HandCrawler::RightHand_HandCrawler(
	RightProjectileHand& aRightHand)
	: HandCrawler(aRightHand),
	myRightHand(aRightHand)
{
}

FB::RightProjectileHand& FB::RightHand_HandCrawler::GetRightHand()
{
	return myRightHand;
}