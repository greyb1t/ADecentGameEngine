#include "pch.h"
#include "ItemsHowToPlay.h"

#include "Game/Items/ItemManager.h"

#include "Engine/GameObject/GameObject.h"
#include "Engine/GameObject/Prefab/GameObjectPrefab.h"

#include "Engine/GameObject/Components/TextComponent.h"
#include "Engine/GameObject/Components/SpriteComponent.h"
#include "Engine/GameObject/Components/2DComponents/ButtonComponent.h"

void ItemsHowToPlay::Start()
{
	if (!myItemButtonPrefab->IsValid())
	{
		LOG_ERROR(LogType::Game) << "No Itembutton was assigned.";
		return;
	}

	InitRefs();
	InitItemDescs();
	InitButtonCallbacks();
}

void ItemsHowToPlay::Reflect(Engine::Reflector& aReflector)
{
	aReflector.Reflect(myItemButtonPrefab, "Button Prefab");

	aReflector.Reflect(myImageRef, "Image Ref");
	aReflector.Reflect(myTitleRef, "Title Ref");
	aReflector.Reflect(myDescriptionRef, "Description Ref");

	aReflector.Reflect(myDebugDraw, "Debug Draw?");
	
	if (aReflector.Reflect(myDebugAmount, "Debug Amount") & Engine::ReflectorResult_Changed)
	{
		if (myDebugAmount < 1)
		{
			myDebugAmount = 1;
		}
	}

	aReflector.Reflect(myStartPos, "Start Position");
	aReflector.Reflect(myPadding, "Padding");

	if (aReflector.Reflect(myColumnSize, "Column Size") & Engine::ReflectorResult_Changed)
	{
		if (myColumnSize < 1)
		{
			myColumnSize = 1;
		}
	}
}

void ItemsHowToPlay::Execute(Engine::eEngineOrder aOrder)
{
	if (!myDebugDraw)
		return;

	const auto& targetSize = GetEngine().GetTargetSize();

	Vec2f pos = myStartPos;
	float yPos = 0.0f;
	float xPos = 0.0f;

	for (int i = 0; i < myDebugAmount; i++)
	{
		if (i != 0)
		{
			xPos += myPadding.x / targetSize.x;
		}

		if (i != 0 && i % myColumnSize == 0)
		{
			yPos += myPadding.y / targetSize.y;
			xPos = 0.0f;
		}
		
		GDebugDrawer->DrawRectangle2D(
			DebugDrawFlags::Always,
			Vec2f(pos.x + xPos, pos.y + yPos),
			Vec2f((200.f / targetSize.x) * .2f, (200.f / targetSize.y) * .2f));
	}
}

void ItemsHowToPlay::OnEnable()
{
	for (auto& item : myItemDescs)
	{
		item.myTransform->GetGameObject()->SetActive(true);
	}
}

void ItemsHowToPlay::OnDisable()
{
	for (auto& item : myItemDescs)
	{
		item.myTransform->GetGameObject()->SetActive(false);
	}

	if (myImageSprite)
	{
		myImageSprite->ClearSprite();
	}

	if (myTitleText)
	{
		myTitleText->SetText("");
	}

	if (myDescriptionText)
	{
		myDescriptionText->SetText("");
	}
}

void ItemsHowToPlay::InitRefs()
{
	if (myTitleRef.IsValid())
	{
		myTitleText = myTitleRef.Get()->GetComponent<Engine::TextComponent>();
	}

	if (myDescriptionRef.IsValid())
	{
		myDescriptionText = myDescriptionRef.Get()->GetComponent<Engine::TextComponent>();
	}

	if (myImageRef.IsValid())
	{
		myImageSprite = myImageRef.Get()->GetComponent<Engine::SpriteComponent>();
	}
}

void ItemsHowToPlay::InitItemDescs()
{
	const auto& items = Main::GetItemManager().GetAllItems();

	std::map<eRarity, std::vector<Item*>> itemsByRarity;
	for (auto item : items)
	{
		itemsByRarity[item->GetRarity()].push_back(item);
	}

	for (auto item : itemsByRarity[eRarity::Common])
	{
		AddItemDesc(item);
	}

	for (auto item : itemsByRarity[eRarity::Rare])
	{
		AddItemDesc(item);
	}

	for (auto item : itemsByRarity[eRarity::Legendary])
	{
		AddItemDesc(item);
	}

	Vec2f pos = myStartPos;
	float yPos = 0.0f;
	float xPos = 0.0f;

	const auto& targetSize = GetEngine().GetTargetSize();
	int i = 0;
	for (auto& item : myItemDescs)
	{
		if (i != 0)
		{
			xPos += myPadding.x / targetSize.x;
		}

		if (i != 0 && i % myColumnSize == 0)
		{
			yPos += myPadding.y / targetSize.y;
			xPos = 0.0f;
		}

		auto transform = item.myTransform;
		auto position = transform->GetPosition();
		transform->SetPosition({ pos.x + xPos, pos.y + yPos, position.z });
		i++;
	}
}

void ItemsHowToPlay::InitButtonCallbacks()
{
	UI::AddEvent("EnterItem", [this](GameObject* aButtonObject) { OnEnterItem(aButtonObject); });
	UI::AddEvent("ExitItem", [this](GameObject* aButtonObject) { OnExitItem(aButtonObject); });
}

void ItemsHowToPlay::OnEnterItem(GameObject* aButtonObject)
{
	if (!aButtonObject)
		return;

	const std::string& name = aButtonObject->GetName();

	ItemDesc* desc = nullptr;
	for (auto& item : myItemDescs)
	{
		if (item.myName == name)
		{
			desc = &item;
			break;
		}
	}

	if (desc)
	{
		if (myImageSprite)
		{
			myImageSprite->SetSprite(desc->mySpritePath);
		}

		if (myTitleText)
		{
			myTitleText->SetText(desc->myName);
		}

		if (myDescriptionText)
		{
			myDescriptionText->SetText(desc->myLongDesc);
		}
	}
}

void ItemsHowToPlay::OnExitItem(GameObject* aButtonObject)
{
	if (myImageSprite)
	{
		myImageSprite->ClearSprite();
	}

	if (myTitleText)
	{
		myTitleText->SetText("");
	}

	if (myDescriptionText)
	{
		myDescriptionText->SetText("");
	}
}

void ItemsHowToPlay::AddItemDesc(Item* anItem)
{
	Engine::GameObjectPrefab& prefab = myItemButtonPrefab->Get();
	GameObject& gameObject = prefab.Instantiate(*myGameObject->GetScene());
	gameObject.SetName(anItem->GetName());

	const auto& path = anItem->GetSpritePath();

	if (auto sprite = gameObject.GetComponent<Engine::SpriteComponent>())
	{
		sprite->SetSprite(path);
	}

	if (auto button = gameObject.GetComponent<Engine::ButtonComponent>())
	{
		button->SetDefaultSprite(path);
		button->SetHoverSprite(path);
	}

	ItemDesc desc;
	desc.myName = anItem->GetName();
	desc.myShortDesc = anItem->GetShortDesc();
	desc.myLongDesc = anItem->GetDesc();
	desc.mySpritePath = anItem->GetSpritePath();
	desc.myTransform = &gameObject.GetTransform();

	if (!desc.myTransform->GetChildren().empty())
	{
		auto child = desc.myTransform->GetChildByIndex(0);
		if (auto sprite = child->GetGameObject()->GetComponent<Engine::SpriteComponent>())
		{
			switch (anItem->GetRarity())
			{
			case eRarity::Common:
				sprite->SetColor(COLOR_COMMON);
				break;

			case eRarity::Rare:
				sprite->SetColor(COLOR_RARE);
				break;

			case eRarity::Legendary:
				sprite->SetColor(COLOR_LEGENDARY);
				break;

			default:
				break;
			}
		}
	}

	myItemDescs.push_back(desc);
}
