#include "pch.h"
#include "ChestComponent.h"

#include "Engine/GameObject/GameObject.h"
#include "Game/Components/InteractableManager.h"
#include "Game/Components/Item/ItemChoice.h"

#include "Engine/GameObject/Prefab/GameObjectPrefab.h"
#include "Engine/Shortcuts.h"
#include "Engine/Reflection/Reflector.h"
#include "Components/WalletComponent.h"
#include "Items/ItemManager.h"

#include "HUD/HUDInteraction.h"

#include "Engine/ResourceManagement/Resources/VFXResource.h"
#include "Engine/GameObject/Components/VFXComponent.h"
#include "Engine\GameObject\Components\ModelComponent.h"
#include "Engine\Animation/AnimationController.h"
#include "Engine\GameObject\Components\AnimatorComponent.h"
#include "Engine/GameObject/Components/AnimatorComponent.h"
#include "Engine/Animation/AnimationController.h"
#include "Engine/GameObject/Components/AudioComponent.h"
#include "Components\PlayerShield.h"

void ChestComponent::Awake()
{
	ObserveInteract([&](){Open();});
	ObserveHoverEnter([&](){OnHoverEnter();});
	ObserveHoverExit([&](){OnHoverExit();});

	Interactable::Awake();
}

void ChestComponent::Start()
{
	{
		myPriceTag = GetTransform().GetChildByGameObjectName("Pricetag")->GetComponent<Engine::TextComponent>();
		std::string price = "$" + std::to_string(GetPrice());

		if (GetPrice() == 0)
		{
			price = "Free";
		}

		myPriceTag->SetText(price);
		myStartScale = myPriceTag->GetTransform().GetScale();
	}

	GameObject* beamObj = GetTransform().GetChildByGameObjectName("Beam");
	if (beamObj)
	{
		myBeamObject = beamObj->GetWeak();
		Vec3f pos = beamObj->GetTransform().GetPosition();
		beamObj->GetTransform().RemoveParent();
		beamObj->GetTransform().SetPosition(pos);
		beamObj->GetTransform().SetParent(&myGameObject->GetTransform());
	}

	myAudio = myGameObject->AddComponent<Engine::AudioComponent>();
	myAudio->AddEvent("OpenCommon", "event:/SFX/ITEM/Common");
	myAudio->AddEvent("OpenRare", "event:/SFX/ITEM/Uncommon");
	myAudio->AddEvent("OpenLegendary", "event:/SFX/ITEM/Legendary");
	myAudio->AddEvent("Pickup", "event:/SFX/ITEM/Pickup");
	myAudio->AddEvent("PickupSpecial", "event:/SFX/ITEM/EgarsAxePickup");

	auto animation = myGameObject->GetComponent<Engine::AnimatorComponent>();
	if(animation)
	{
		myAnimation = &animation->GetController();
		myAnimation->AddStateOnExitCallback("Base Layer", "Open", [&]() { FinishedOpening(); });
		myAnimation->AddEventCallback("StartVFX", [&]() { StartVFX(); });
	}
	else
	{
		LOG_ERROR(LogType::Game) << "Could not find AnimationComponent";
	}

	Interactable::Start();
}

void ChestComponent::Reflect(Engine::Reflector& aReflector)
{
	Interactable::Reflect(aReflector);
	aReflector.Reflect(myItemChoicePrefab, "Item Choice Prefab");
	aReflector.Reflect(myShieldPrefab, "Shield Prefab");

	aReflector.Reflect(myPriceDistance, "Price Show Distance");

	aReflector.Header("VFX");
	aReflector.Reflect(myVFXRef, "Open VFX");
	aReflector.Reflect(myVFXRotation, "VFX Rotation");

	aReflector.Header("Stats");
	aReflector.Reflect(myPrice, "Cost to open"); 

	aReflector.Reflect(myIsRandomDrop, "Random Drop Type");

	if (myIsRandomDrop)
	{
		aReflector.Reflect(myRandomValues[0], "Common Drop Chance");
		aReflector.Reflect(myRandomValues[1], "Rare Drop Chance");
		aReflector.Reflect(myRandomValues[2], "Legendary Drop Chance");
	}
	else
	{
		aReflector.Reflect(myRarePriceMultiplier, "Cost Multiplier");
		aReflector.Reflect(myRandomValues[1], "Rare Drop Chance");
		aReflector.Reflect(myRandomValues[2], "Legendary Drop Chance");
	}
}

void ChestComponent::Execute(Engine::eEngineOrder aOrder)
{
	UpdatePrice();

	if (myIsOpened)
	{
		float itemHeight = 200.0f;
		Vec3f right = myGameObject->GetScene()->GetMainCameraGameObject()->GetTransform().Right() * 100.f;

		Vec3f firstTargetPosition = GetGameObject()->GetTransform().GetPosition() + Vec3f{ 0.f, itemHeight, 0.f } - right;
		Vec3f secondTargetPosition = GetGameObject()->GetTransform().GetPosition() + Vec3f{ 0.f, itemHeight, 0.f };
		Vec3f thirdTargetPosition = GetGameObject()->GetTransform().GetPosition() + Vec3f{ 0.f, itemHeight, 0.f } + right;

		myFirstObject->SetTargetPosition(firstTargetPosition);
		mySecondObject->SetTargetPosition(secondTargetPosition);
		myThirdObject->SetTargetPosition(thirdTargetPosition);
	}
}

void ChestComponent::OnItemSelected(Item* aItem)
{
	myIsOpened = false;

	myFirstObject->Close();
	mySecondObject->Close();
	myThirdObject->Close();

	if (aItem->GetName() == "Egars Axe")
	{
		myAudio->PlayEvent("PickupSpecial");
	}
	else
	{
		myAudio->PlayEvent("Pickup");
	}

	//myFirstObject->GetGameObject()->Destroy();
	//mySecondObject->GetGameObject()->Destroy();
	//myThirdObject->GetGameObject()->Destroy();

	myFirstObject = nullptr;
	mySecondObject = nullptr;
	myThirdObject = nullptr;

	if (myShield)
		myShield->Deactivate();
}

int ChestComponent::GetPrice()
{
	return static_cast<int>(static_cast<float>(myPrice) * myRarePriceMultiplier);
}

void ChestComponent::UpdatePrice()
{
	{
		Vec3f targetPosition = myGameObject->GetScene()->GetMainCamera().GetTransform().GetPosition();
		Vec3f position = myPriceTag->GetTransform().GetPosition();

		float distance = (Main::GetPlayer()->GetTransform().GetPosition() - position).Length();

		Vec3f diff = targetPosition - position;
		diff.y = 0.0f;

		myPriceTag->GetTransform().LookAt(position - diff);

		bool shouldShow = distance < myPriceDistance;
		myFadeTimer += (shouldShow && myShouldUpdatePrice) ? Time::DeltaTime : -Time::DeltaTime;
		myFadeTimer = std::clamp(myFadeTimer, 0.0f, myFadeTime);

		float percent = myFadeTimer / myFadeTime;
		//myPriceTag->SetAlpha(percent);
		Vec3f scale = C::Lerp(Vec3f(), myStartScale, percent);
		myPriceTag->GetTransform().SetScale(scale);

		if (percent <= 0.0f && myPriceTag->IsActive())
		{
			myPriceTag->SetActive(false);
		}
		else if (percent > 0.0f && !myPriceTag->IsActive())
		{
			myPriceTag->SetActive(true);
		}
	}

	{
		myDeclineTimer += Time::DeltaTime;
		myDeclineTimer = std::clamp(myDeclineTimer, 0.0f, myDeclineTime);

		float percent = myDeclineTimer / myDeclineTime;

		myPriceTag->SetColor(Vec4f(1.0f, percent, percent, 1.0f));
	}
}

void ChestComponent::Open()
{
	if (!myHasStartedOpenening && Main::GetPlayer()->GetComponent<WalletComponent>()->TryBuy(GetPrice()))
	{
		myHasStartedOpenening = true;

		myRarity = GetRandomRarity();
		OpenChestSound(myRarity);

		auto audioComp = myGameObject->GetComponent<Engine::AudioComponent>();
		myAnimation->Trigger("Open");

		myShouldUpdatePrice = false;
		myPriceTag->SetActive(false);
		if (auto shared = myBeamObject.lock())
		{
			shared->SetActive(false);
		}

		if (myShieldPrefab && myShieldPrefab->IsValid())
		{
			myShield = myShieldPrefab->Get().Instantiate(*GetGameObject()->GetScene()).GetComponent<PlayerShield>();
			myShield->GetTransform().SetParent(&GetTransform());
			myShield->GetTransform().SetPositionLocal(Vec3f());

			myShield->SetDeactivateCallback([&]() { myShield = nullptr; });
		}
	}
	else if(!myHasStartedOpenening)
	{
		myDeclineTimer = 0.0f;
		myAnimation->Trigger("NoCash");
		myHUDInteraction->Decline();
	}
}

void ChestComponent::OnHoverEnter()
{
	std::string price = "Open Chest";
	if (GetPrice() != 0)
	{
		price += " $" + std::to_string(GetPrice());
	}

	myHUDInteraction->SetInteractionName(price);
}

void ChestComponent::OnHoverExit()
{
}

void ChestComponent::FinishedOpening()
{
	myIsOpened = true;
	myFirstObject = myItemChoicePrefab->Get().Instantiate(*GetGameObject()->GetScene()).GetComponent<ItemChoice>();
	mySecondObject = myItemChoicePrefab->Get().Instantiate(*GetGameObject()->GetScene()).GetComponent<ItemChoice>();
	myThirdObject = myItemChoicePrefab->Get().Instantiate(*GetGameObject()->GetScene()).GetComponent<ItemChoice>();

	Vec3f firstTargetPosition = GetGameObject()->GetTransform().GetPosition() + Vec3f{ 0.f, 100.f, 0.f } + Main::GetPlayer()->GetTransform().Right() * -100.f;
	Vec3f secondTargetPosition = GetGameObject()->GetTransform().GetPosition() + Vec3f{ 0.f, 100.f, 0.f };
	Vec3f thirdTargetPosition = GetGameObject()->GetTransform().GetPosition() + Vec3f{ 0.f, 100.f, 0.f } + Main::GetPlayer()->GetTransform().Right() * 100.f;

	myFirstObject->Open(0, this, GetGameObject()->GetTransform().GetPosition(), firstTargetPosition);
	mySecondObject->Open(1, this, GetGameObject()->GetTransform().GetPosition(), secondTargetPosition);
	myThirdObject->Open(2, this, GetGameObject()->GetTransform().GetPosition(), thirdTargetPosition);

	RandomizeItems();
	SetCanInteract(false);
}

void ChestComponent::StartVFX()
{
	if (!myVFXRef->IsValid())
		return;

	auto child = myGameObject->GetScene()->AddGameObject<GameObject>();
	child->GetTransform().SetParent(&myGameObject->GetTransform());
	child->GetTransform().SetPositionLocal(Vec3f(0.0f, 50.0f, 0.0f));
	child->GetTransform().SetRotationLocal(myVFXRotation);

	myVFX = child->AddComponent<Engine::VFXComponent>(myVFXRef->Get());
	myVFX->Play();
}

void ChestComponent::RandomizeItems()
{
	float totalRarityChance = 0.f;
	for (float val : myRandomValues)
	{
		totalRarityChance += val;
	}

	float chestDropValue = myIsRandomDrop ? 0.0f : myRandomValues[0];

	float randomizedValue = Random::RandomFloat(chestDropValue, totalRarityChance);
	std::vector<Item*> selectedItems;
	selectedItems.reserve(3); 
	for (int itemIndex = 0; itemIndex < 3; ++itemIndex)
	{
		float iterationValue = 0.f;
		for (size_t valIndex = 0; valIndex < myRandomValues.size(); ++valIndex)
		{
			iterationValue += myRandomValues[valIndex];
			if (randomizedValue <= iterationValue)
			{
				Item* item = Main::GetItemManager().GetRandomItem(myRarity, selectedItems);
				selectedItems.push_back(item);
				LOG_ERROR(LogType::Bjorn) << item->GetRarity()._to_string() << "!";
				break;
			}
		}
	}

	myFirstObject->SetItem(selectedItems[0]);
	mySecondObject->SetItem(selectedItems[1]);
	myThirdObject->SetItem(selectedItems[2]);
}

void ChestComponent::OpenChestSound(eRarity aRarity)
{
	switch (aRarity)
	{
	case eRarity::Common:
		myAudio->PlayEvent("OpenCommon");
		break;

	case eRarity::Rare:
		myAudio->PlayEvent("OpenRare");
		break;

	case eRarity::Legendary:
		myAudio->PlayEvent("OpenLegendary");
		break;
	}
}

eRarity ChestComponent::GetRandomRarity()
{
	float randomVal = Random::RandomFloat(0.0f, myRandomValues[0] + myRandomValues[1] + myRandomValues[2]);

	if (randomVal < myRandomValues[2])
	{
		return eRarity::Legendary;
	}
	else if (randomVal < myRandomValues[1])
	{
		return eRarity::Rare;
	}

	if (myIsRandomDrop)
	{
		return eRarity::Common;
	}

	return eRarity::Rare;
}
