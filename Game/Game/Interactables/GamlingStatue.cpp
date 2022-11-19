#include "pch.h"
#include "GamlingStatue.h"

#include "Engine/Engine.h"
#include "Engine\GameObject\GameObject.h"
#include "Engine/GameObject/Prefab/GameObjectPrefab.h"
#include "Game/Components/Item/ItemChoice.h"
#include "Items/ItemManager.h"
#include "Components/WalletComponent.h"
#include "Engine\GameObject\Components\CameraShakeComponent.h"
#include "Engine/GameObject/Components/ModelComponent.h"
#include "Engine/ResourceManagement/Resources/VFXResource.h"
#include "Engine/GameObject/Components/VFXComponent.h"
#include "Engine/GameObject/Components/AudioComponent.h"
#include "HUD/HUDInteraction.h"

void GamlingStatue::Awake()
{
	ObserveInteract([&] { Gamble(); });
	ObserveHoverEnter([&] { OnHoverEnter(); });
	ObserveHoverExit([&] { OnHoverExit(); });

	Interactable::Awake();
	
	SetIsSandy(myIsSandy);

	myAudio = myGameObject->AddComponent<Engine::AudioComponent>();
	myAudio->AddEvent("Win", "event:/SFX/INT/ShrineSucces");
	myAudio->AddEvent("Fail", "event:/SFX/INT/ShrineFail");
}

void GamlingStatue::Start()
{
	Interactable::Start();

	const std::string& levelName = GameManager::GetInstance()->GetLevelName();
	if (levelName == "Assets/Scenes/UEDPIE_0_LVL03_DesertDunes" || levelName == "Assets/Scenes/UEDPIE_0_LVL04_DesertHills")
	{
		SetIsSandy(true);
	}
	
	myPriceTag = GetTransform().GetChildByGameObjectName("Pricetag")->GetComponent<Engine::TextComponent>();
	myPriceTag->SetText("$" + std::to_string(myPrice));
	myStartScale = myPriceTag->GetTransform().GetScale();

	if (myIdleVFXRef && myIdleVFXRef->IsValid())
	{
		myIdleVFX = myGameObject->AddComponent<Engine::VFXComponent>(myIdleVFXRef->Get());
		myIdleVFX->Play();
	}
}

void GamlingStatue::Reflect(Engine::Reflector& aReflector)
{
	Interactable::Reflect(aReflector);
	aReflector.Reflect(myItemChoicePrefab, "Item Choice Prefab");
	aReflector.Reflect(myPriceDistance, "Price Show Distance");

	aReflector.Header("VFX");
	aReflector.Reflect(myWinVFXRef, "Win VFX");
	aReflector.Reflect(myFailVFXRef, "Fail VFX");
	aReflector.Reflect(myIdleVFXRef, "Idle VFX");

	aReflector.Header("Drop Chance (0 - 10)");
	aReflector.Reflect(myItemDropChance, "Drop Chance");
	aReflector.Reflect(myTryChanceDecrease, "Try Chance Decrease Multiplier");
	aReflector.Reflect(myRareChance, "Rare Drop Chance");
	aReflector.Reflect(myLegendaryChance, "Legendary Drop Chance");

	aReflector.Header("Price");
	aReflector.Reflect(myPrice, "Price");
	aReflector.Reflect(myTryPirceIncrease, "Try Price Increase Multiplier");
}

void GamlingStatue::Execute(Engine::eEngineOrder aOrder)
{
	UpdatePrice();

	myCantInteractTimer += Time::DeltaTime;

	if (!myCanInteract && myCantInteractTimer > myCantInteractTime)
	{
		SetCanInteract(true);
	}
}

void GamlingStatue::SetIsSandy(bool aIsSandy)
{
	myIsSandy = aIsSandy;

	auto model = myGameObject->GetComponent<Engine::ModelComponent>();
	if (model && myIsSandy)
	{
		model->SetMaterial("M_Shrine_Sand");
	}
	else
	{
		model->SetMaterial("M_Shrine");
	}
}

void GamlingStatue::UpdatePrice()
{
	Vec3f targetPosition = myGameObject->GetScene()->GetMainCamera().GetTransform().GetPosition();
	Vec3f position = myPriceTag->GetTransform().GetPosition();

	float distance = (Main::GetPlayer()->GetTransform().GetPosition() - position).Length();

	Vec3f diff = targetPosition - position;
	diff.y = 0.0f;

	//myPriceTag->GetTransform().LookAt(position - diff);

	bool shouldShow = distance < myPriceDistance;
	myFadeTimer += shouldShow ? Time::DeltaTime : -Time::DeltaTime;
	myFadeTimer = std::clamp(myFadeTimer, 0.0f, myFadeTime);

	float percent = myFadeTimer / myFadeTime;
	//myPriceTag->SetAlpha(percent);
	Vec3f scale = C::Lerp(Vec3f(), myStartScale, percent);
	myPriceTag->GetTransform().SetScale(scale);

	//LOG_INFO(LogType::Simon) << percent;

	if (percent <= 0.0f && myPriceTag->IsActive())
	{
		myPriceTag->SetActive(false);
	}
	else if (percent > 0.0f && !myPriceTag->IsActive())
	{
		myPriceTag->SetActive(true);
	}
}

void GamlingStatue::Gamble()
{
	if (!Main::GetPlayer()->GetComponent<WalletComponent>()->TryBuy(myPrice))
	{
		myHUDInteraction->Decline();
		return;
	}

	float random = Random::RandomFloat(0.0f, 10.0f);
	if (myItemDropChance > random)
	{
		SpawnItem();
		StartWinVFX();

		myItemDropChance *= myTryChanceDecrease;

		myAudio->PlayEvent("Win");
	}
	else
	{
		StartFailVFX();
		myAudio->PlayEvent("Fail");
	}

	myPrice = static_cast<int>(static_cast<float>(myPrice) * myTryPirceIncrease);
	myPriceTag->SetText("$" + std::to_string(myPrice));

	myHUDInteraction->SetInteractionName("Try Shrine $" + std::to_string(myPrice));
}

void GamlingStatue::OnHoverEnter()
{
	myHUDInteraction->SetInteractionName("Try Shrine $" + std::to_string(myPrice));
}

void GamlingStatue::OnHoverExit()
{

}

void GamlingStatue::SpawnItem()
{
	//Set Item
	float rarityChance = Random::RandomFloat(0.0f, 10.0f);
	eRarity rarity = eRarity::Common;
	if (rarityChance < myLegendaryChance)
	{
		rarity = eRarity::Legendary;
	}
	else if (rarityChance < myRareChance)
	{
		rarity = eRarity::Rare;
	}

	Item* item = Main::GetItemManager().GetRandomItem(rarity, std::vector<Item*>());


	//Set Target Position
	Vec3f targetPosition;

	size_t tries = 5;
	for (size_t i = 0; i <= tries; i++)
	{
		targetPosition = GetTransform().GetPosition() - GetTransform().Forward() * 150.0f;
		targetPosition += GetTransform().Forward() * Random::RandomFloat(-200.0f, 50.0f);
		targetPosition += GetTransform().Right() * Random::RandomFloat(-200.0f, 200.0f);

		if (i == tries)
			break;

		RayCastHit hit;
		if (myGameObject->GetScene()->RayCast(targetPosition + Vec3f(0.0f, 3000.0f, 0.0f), {0.f, -1.f, 0.f}, 15000.f, eLayer::ALL ^ eLayer::GROUND, hit, false))
		{
			targetPosition = hit.Position;
			targetPosition.y += 150.0f;
			LOG_INFO(LogType::Simon) << "FOUND GROUND POS";
			break;
		}
	}


	//Create Item Choice
	auto itemChioce = myItemChoicePrefab->Get().Instantiate(*GetGameObject()->GetScene()).GetComponent<ItemChoice>();
	itemChioce->SetItem(item);
	itemChioce->SetLerpSpeed(1.5f);
	itemChioce->DestroyOnSelect();
	itemChioce->SetInteractRange(itemChioce->GetInteractRange() * 1.5f);

	itemChioce->Open(0, nullptr, GetGameObject()->GetTransform().GetPosition(), targetPosition);


	SetCanInteract(false);
	myCantInteractTimer = 0.0f;
}

void GamlingStatue::StartWinVFX()
{
	if (!myWinVFXRef->IsValid())
		return;

	auto vfxObject = myGameObject->GetScene()->AddGameObject<GameObject>();
	vfxObject->GetTransform().SetParent(&myGameObject->GetTransform());
	vfxObject->GetTransform().SetPositionLocal(Vec3f(0.0f, 50.0f, 0.0f));

	myWinVFX = vfxObject->AddComponent<Engine::VFXComponent>(myWinVFXRef->Get());
	myWinVFX->Play();
	myWinVFX->AutoDestroy();
}

void GamlingStatue::StartFailVFX()
{
	auto mainCam = GetGameObject()->GetScene()->GetMainCameraGameObject();
	auto camShake = mainCam->GetComponent<Engine::CameraShakeComponent>();
	camShake->AddPerlinShake("GambleFailShake");


	if (!myFailVFXRef->IsValid())
		return;


	auto vfxObject = myGameObject->GetScene()->AddGameObject<GameObject>();
	vfxObject->GetTransform().SetParent(&myGameObject->GetTransform());
	vfxObject->GetTransform().SetPositionLocal(Vec3f(0.0f, 50.0f, 0.0f));

	myFailVFX = vfxObject->AddComponent<Engine::VFXComponent>(myFailVFXRef->Get());
	myFailVFX->Play();
	myFailVFX->AutoDestroy();
}
