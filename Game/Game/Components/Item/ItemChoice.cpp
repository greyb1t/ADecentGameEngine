#include "pch.h"
#include "ItemChoice.h"

#include "Engine/GameObject/GameObject.h"
#include "Game/Components/InteractableManager.h"
#include "Game/Components/Item/ChestComponent.h"

#include "Common\CommonUtilities.h"

#include "MainSingleton/MainSingleton.h"
#include "Items/ItemManager.h"
#include "Items/ItemTypes.h"
#include "Prototype/RorPlayer.h"

#include "Game/Components/InventoryComponent.h"

#include "HUD/HUDHandler.h"
#include "HUD/HUDItemDescription.h"
#include "HUD/HUDInteraction.h"

#include "Engine\GameObject\Components\SpriteComponent.h"

void ItemChoice::Awake()
{
	ObserveInteract([&] { Select(); });
	ObserveHoverEnter([&] { HoverEnter(); });
	ObserveHoverExit([&] { HoverExit(); });
	InteractableManager::Instance().Register(this);

	SetCanInteract(false);
	SetDisplayInteractMessage(false);
}

void ItemChoice::Start()
{
	auto player = Main::GetPlayer();
	if (player)
	{
		myHUDItemDescription = player->GetComponent<HUDHandler>()->GetItemDescriptionBox();
		myHUDInteraction = player->GetComponent<HUDHandler>()->GetInteractionBox();
	}

	auto sprite = myGameObject->GetComponent<Engine::SpriteComponent>();
	sprite->SetSprite(myItem->GetSpritePath());
	sprite->SetMaterial("Default3D");
	sprite->SetSortOrder(0);
	sprite->SetIs3D(true);

	GetTransform().SetScale(myScale);
}

void ItemChoice::Execute(Engine::eEngineOrder aOrder)
{
	if (myLerpAmount >= 1.f)
	{
		if (!myIsReturning)
		{
			SetCanInteract(true);
		}
		else
		{
			GetGameObject()->Destroy();
		}
	}

	myLerpAmount += Time::DeltaTime * myLerpSpeed;

	const Vec3f newPosition = [&]() -> Vec3f
	{
		float lerpAmount = CU::Clamp<float>(0.f, 1.f, myLerpAmount);

		if (myWasSelected)
		{
			// NOTE(Ajlin): This is for the y-motion when selecting the item
			if (!myPontusBool)
			{
				float v = 2.f * lerpAmount - 1.f;
				float yFactor = -(v * v) + 1.f;
				static constexpr float yOffset = 400.f;
				return CU::Lerp<Vec3f>(myOriginalPosition, myTargetPosition + Vec3f{ 0.f, yOffset * yFactor, 0.f }, lerpAmount);
			}
			return CU::Lerp<Vec3f>(myOriginalPosition, myTargetPosition, lerpAmount);
		}

		if (!myPontusBool)
		{
			// NOTE(Ajlin): Function for the bouncing effect
			static constexpr float c1 = 1.70158;
			static constexpr float c3 = c1 + 1;

			const float lMinus1 = lerpAmount - 1;
			const float pow2 = lMinus1 * lMinus1;
			const float pow3 = pow2 * lMinus1;

			float t = 1 + c3 * pow3 + c1 * pow2;

			return CU::Lerp<Vec3f>(myOriginalPosition, myTargetPosition, t);
		}
		return CU::Lerp<Vec3f>(myOriginalPosition, myTargetPosition, lerpAmount); 
	}();

	const Vec3f hoverOffsetTarget{ 0.f, 25.f, 0.f };

	myHoverLerpAmount += Time::DeltaTime * 8.f * myHoverLerpDirection;
	myHoverLerpAmount = CU::Clamp(0.f, 1.f, myHoverLerpAmount);
	Vec3f hoverOffset = CU::Lerp<Vec3f>({ 0.f, 0.f, 0.f }, hoverOffsetTarget, myHoverLerpAmount);

	if (myIsReturning && !myWasSelected)
	{
		float scaleT = myHoverLerpAmount;
		scaleT = 1 - scaleT;
		scaleT *= scaleT * scaleT;
		GetTransform().SetScale(CU::Lerp<Vec3f>({ myScale, myScale, myScale }, { 0, 0, 0 }, scaleT));
	}

	GetGameObject()->GetTransform().SetPosition(newPosition + hoverOffset);
}

void ItemChoice::Open(const uint32_t aIndex, ChestComponent* aChest, const Vec3f& aOriginalPosition, const Vec3f& aTargetPosition)
{
	myIndex = aIndex;
	myChest = aChest;
	myOriginalPosition = aOriginalPosition;
	myTargetPosition = aTargetPosition;
	myLerpAmount = -static_cast<float>(myIndex) * 0.5f;
}

void ItemChoice::Close()
{
	HoverExit();

	myLerpAmount = -static_cast<float>(myIndex) * 0.5f;
	CU::Swap(myOriginalPosition, myTargetPosition);

	myIsReturning = true;
	SetCanInteract(false);

	if (myWasSelected)
	{
		myTargetPosition = Main::GetPlayer()->GetTransform().GetPosition() + Vec3f{ 0.f, 100.f, 0.f };
		myLerpAmount = 0.f;
	}
}

void ItemChoice::HoverEnter()
{
	myHoverLerpAmount = 0.f;
	myHoverLerpDirection = 1.f;

	myHUDItemDescription->ShowShortDesc(true);
	myHUDItemDescription->SetName(myItem->GetName());
	myHUDItemDescription->SetShortDesc(myItem->GetShortDesc());

	myHUDInteraction->SetInteractionName("Choose Item");
}

void ItemChoice::HoverExit()
{
	myHoverLerpAmount = 1.f;
	myHoverLerpDirection = -1.f;

	myHUDItemDescription->ShowShortDesc(false);
	myHUDInteraction->ShowDesc(false);
}

void ItemChoice::SetTargetPosition(const Vec3f aPosition)
{
	myTargetPosition = aPosition;
}

void ItemChoice::SetItem(Item* anItem)
{
	myItem = anItem;
}

void ItemChoice::DestroyOnSelect()
{
	myDestroyOnSelect = true;
}

void ItemChoice::SetLerpSpeed(float aLerpSpeed)
{
	myLerpSpeed = aLerpSpeed;
}

void ItemChoice::SetPontusBoolToTrue()
{
	myPontusBool = true;
}

void ItemChoice::Select()
{
	InventoryComponent* inventory = Main::GetPlayer()->GetComponent<InventoryComponent>();

	if (inventory && myItem)
	{
		inventory->AddItem(*myItem);
	}

	myWasSelected = true;
	if(myChest)
		myChest->OnItemSelected(myItem);

	myHUDItemDescription->PickupItem();
	myHUDItemDescription->SetName(myItem->GetName());
	myHUDItemDescription->SetDesc(myItem->GetDesc());

	if (myDestroyOnSelect)
	{
		myHUDItemDescription->ShowShortDesc(false);
		myHUDInteraction->ShowDesc(false);
		myGameObject->Destroy();
	}
}