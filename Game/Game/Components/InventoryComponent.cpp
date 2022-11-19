#include "pch.h"
#include "InventoryComponent.h"
#include "Items/Item.h"
#include "Items/ItemManager.h"
#include "Engine/Reflection/Reflector.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/GameObject/Components/TextComponent.h"

#include "Engine/GameObject/Components/SpriteComponent.h"
#include "UI\PauseMenuComponent.h"

#include "Engine/ResourceManagement/Resources/VFXResource.h"
#include "Engine/GameObject/Components/VFXComponent.h"

InventoryComponent::~InventoryComponent()
{
	for (auto* item : myItems)
	{
		delete item;
		item = nullptr;
	}
}

void InventoryComponent::Start()
{
	auto HUD = GetTransform().GetParent()->GetChildByGameObjectName("HUD");
	if (HUD)
	{
		auto pauseMenu = HUD->GetTransform().GetChildByGameObjectName("PauseMenu");
		if (pauseMenu)
		{
			myPauseMenu = pauseMenu->GetComponent<PauseMenuComponent>();
			myPauseMenu->SetPauseListener([&](bool aIsPaused) { ShowItems(aIsPaused); });
			myPauseMenu->SetInventory(this);
		}
	}


	const std::vector<std::pair<Item, int>>& items = Main::GetPlayerItems();
	for (auto& pair : items)
	{
		AddItem(pair.first);
	}
	Main::LoadPlayerHP(); //SET HERE, MUST BE APPLIED AFTER ITEMS HAVE BEEN LOADED
}

void InventoryComponent::Reflect(Engine::Reflector& aReflector)
{
	Component::Reflect(aReflector);

	aReflector.ReflectLambda([this]()
		{
			ImGui::Text("Items");
			ImGui::SameLine();
			if (ImGui::Button("Clear All"))
			{
				Clear();
			}
			ImGui::Separator();
			int i = 0;
			for (auto* item : myItems)
			{
				ImGui::Text(item->GetName().c_str());
				ImGui::SameLine();
				if (ImGui::Button(("Delete##item" + std::to_string(i)).c_str()))
				{
					RemoveItemAtIndex(i);
					break;
				}
				++i;
			}

		});
}

void InventoryComponent::AddItem(const Item& anItem)
{
	int index = -1;
	auto sameItem = [anItem, &index](const Item* testItem) -> bool {++index;  return anItem == *testItem; };
	auto it = std::find_if(myItems.begin(), myItems.end(), sameItem);

	bool stacking = false;
	bool isExisting = false;
	if (it != myItems.end())
	{
		Item*& item = *it;
		if (item->myStackProcChance)
		{
			item->myProcChance += anItem.myProcChance;
			stacking = true;
		}
		for (int i = 0; i < item->myEffects.size(); ++i)
		{
			if (item->myEffects[i]->GetType() == +eItemEventType::Base)
			{
				ItemEffect* effect = static_cast<ItemEffect*>(item->myEffects[i]);
				if (effect->myStackStrength)
				{
					effect->myStrength += static_cast<ItemEffect*>(anItem.myEffects[i])->myStrength;
					stacking = true;
				}
			}
		}
		for (auto* effect : anItem.myEffects)
		{
			if (effect->GetEvent() == +eItemEvent::OnPickup)
			{
				effect->ApplyEffect(*myGameObject, nullptr);
			}
		}

		//Add item to image stack if existing
		item->myStack++;
		//myText[index]->SetActive(true);
		myText[index]->SetText(std::to_string(item->myStack));
	}
	else//Create item image if it doesnt exist
	{
		myItems.push_back(DBG_NEW Item(anItem));
		myItems.back()->Start(); 
		AddSprite(*myItems.back());
	}
}

void InventoryComponent::Clear()
{
	LOG_WARNING(LogType::Simon) << "ASDDASHJHDASHDSHJS CLEARED INVENTORY CLEARED INVENTORY CLEARED INVENTORY CLEARED INVENTORY CLEARED INVENTORY CLEARED INVENTORY CLEARED INVENTORY";

	Main::ClearProgress();
	for (auto* item : myItems)
	{
		delete item;
		item = nullptr;
	}
	myItems.clear(); 

	for (auto image : myImages)
	{
		image.sprite->GetGameObject()->Destroy();
	}
	myImages.clear();

	for (auto* text : myText)
	{
		text->GetGameObject()->Destroy();
	}
	myText.clear();
}

const std::vector<Item*>& InventoryComponent::GetItems() const
{
	return myItems;
}

Item* InventoryComponent::GetSelectedItem(Vec2f aMousePos)
{
	for (auto& image : myImages)
	{
		auto sprite = image.sprite;

		const Vec3f imagePos = sprite->GetTransform().GetPosition();
		const Vec2f halfSize(0.0333335f, 0.05926f); //(128 / 2) / 1920x1080

		if (aMousePos.x > imagePos.x + halfSize.x || aMousePos.x < imagePos.x - halfSize.x)
			continue;

		if (aMousePos.y > imagePos.y + halfSize.y || aMousePos.y < imagePos.y - halfSize.y)
			continue;

		for (auto& item : myItems)
		{
			if(item->myID == image.ID)
				return item;
		}
	}

	return nullptr;
}

const Vec3f& InventoryComponent::GetItemPosition(Item* aItem)
{
	for (auto& image : myImages)
	{
		if (image.ID == aItem->myID)
			return image.sprite->GetTransform().GetPosition();
	}

	return Vec3f();
}

void InventoryComponent::ShowItems(bool aShouldShow)
{
	for (auto& image : myImages)
	{
		image.sprite->SetActive(aShouldShow);
	}

	for (auto& text : myText)
	{
		if (text->GetText() == "1")
			continue;

		text->SetActive(aShouldShow);
	}
}

CU::Vector3f InventoryComponent::ToTextSpace(const CU::Vector3f& aPosition)
{
	CU::Vector2f screenPos(aPosition.x, aPosition.y);
	screenPos = screenPos * 2.f;
	screenPos.x -= 1.f;
	screenPos.y -= 1.f;
	return { screenPos.x, -screenPos.y, 0.f };
}

void InventoryComponent::RemoveItemAtIndex(int anIndex)
{
	delete myItems[anIndex];
	myItems[anIndex] = nullptr;

	myItems.erase(myItems.begin() + anIndex);
	myImages[anIndex].sprite->GetGameObject()->Destroy();
	myImages.erase(myImages.begin() + anIndex);
	myText[anIndex]->GetGameObject()->Destroy();
	myText.erase(myText.begin() + anIndex);

	for (int i = 0; i < myImages.size(); ++i)
	{
		CU::Vector3f position = { myItemStartPos.x + myItemSpacing.x * static_cast<float>(i), myItemStartPos.y, 0.f };
		myImages[i].sprite->GetTransform().SetPosition(position);
		myText[i]->GetTransform().SetPosition(ToTextSpace(position));
	}
}

void InventoryComponent::AddSprite(const Item& anItem)
{
	GameObject* spriteObject = myGameObject->GetScene()->AddGameObject<GameObject>();

	ItemImage image;
	std::string path = anItem.GetSpritePath();
	if (path != "")
	{
		image.sprite = spriteObject->AddComponent<Engine::SpriteComponent>(anItem.GetSpritePath(), 0);
		image.sprite->GetTransform().SetScale(0.18f);
	}

	image.ID = anItem.myID;
	myImages.push_back(image);


	float xPos = myItemStartPos.x + myItemSpacing.x * myRowLength;
	float yPos = myItemStartPos.y + myItemSpacing.y * myRowHeight;

	myRowLength++;
	if (myRowLength > myRowMaxLength)
	{
		myRowLength = 0;
		myRowHeight++;
	}

	CU::Vector3f spritePos = { xPos, yPos, 0.f }; //Magic formula let's goooooo
	spriteObject->GetTransform().SetPosition(spritePos);
	myImages.back().sprite->SetActive(false);
	myImages.back().sprite->SetSortOrder(15);

	GameObject* textObject = myGameObject->GetScene()->AddGameObject<GameObject>();
	Engine::TextComponent* text = textObject->AddComponent<Engine::TextComponent>(std::to_string(anItem.GetStacks()), 0);
	text->SetIsInScreenSpace(true);
	myText.push_back(text);
	textObject->GetTransform().SetScale(0.5f);
	textObject->GetTransform().SetPosition(spritePos + CU::Vector3f(0.019f, -0.023f, 0.f));//(0.025f, 0.04f, 0.f));
	text->SetActive(false);
	text->SetSortOrder(16);
}
