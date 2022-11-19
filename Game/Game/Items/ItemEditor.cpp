#include "pch.h"
#include "ItemEditor.h"
#include "Item.h"
#include "ItemEffect.h"
#include "Engine/GameObject/GameObject.h"
#include "Components/InventoryComponent.h"
#include "Engine/ResourceManagement/Resources/TextureResource.h"
#include "Engine/ResourceManagement/ResourceManager.h"
#include "Engine/Renderer/Texture/Texture2D.h"
#include "Engine/Shortcuts.h"
#include "Engine/Editor/DragDropConstants.h"
#include "Engine/Editor/ImGuiHelper.h"
#include "ItemManager.h"
#include "ItemVFX.h"
#include <fstream>

ItemEditor::ItemEditor()
{
	LoadItems();
}

void ItemEditor::Update()
{
	//MENU BAR

	if (GetEngine().GetInputManager().IsKeyPressed(Common::KeyCode::LeftControl) &&
		GetEngine().GetInputManager().IsKeyDown(Common::KeyCode::I))
	{
		myShouldDisplay = !myShouldDisplay;
	}

	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("Tools"))
		{
			if (ImGui::MenuItem("Item Editor", "Ctrl+I", myShouldDisplay))
			{
				myShouldDisplay = !myShouldDisplay; 
			}

			ImGui::EndMenu();
		}

		ImGui::EndMenuBar();
	}

	//Editor
	if (myShouldDisplay)
	{
		ItemManager& itemManager = Main::GetItemManager(); 
		if (ImGui::Begin("Item Editor"))
		{
			if (ImGui::Button("New item"))
			{
				mySelectedIndex = itemManager.myItems.size();
				CreateNewItem();
			}
			ImGui::SameLine();
			if (ImGui::Button("Delete Item"))
			{
				itemManager.myItems.erase(itemManager.myItems.begin() + mySelectedIndex);
				mySelectedIndex = itemManager.myItems.size() ? CU::Clamp(0, static_cast<int>(itemManager.myItems.size() - 1), mySelectedIndex) : 0;
			}

			if (ImGui::BeginCombo("Items", itemManager.myItems.size() ? itemManager.myItems[mySelectedIndex]->myName.c_str() : "-"))
			{
				for (int i = 0; i < itemManager.myItems.size(); ++i)
				{
					if (ImGui::Selectable((itemManager.myItems[i]->myName + "##" + std::to_string(i)).c_str(), i == mySelectedIndex))
					{
						mySelectedIndex = i;
					}
				}
				ImGui::EndCombo();
			}
			if (ImGui::Button("Add to player"))
			{
				if (Main::GetPlayer())
				{
					Main::GetPlayer()->GetComponent<InventoryComponent>()->AddItem(*itemManager.myItems[mySelectedIndex]);
				}
			}
			ImGui::SameLine();
			if (ImGui::Button("Add 1 of all items"))
			{
				if (Main::GetPlayer())
				{
					for (auto& item : itemManager.myItems)
					{
						Main::GetPlayer()->GetComponent<InventoryComponent>()->AddItem(*item);
					}

				}
			}

			if (itemManager.myItems.size())
			{
				UpdateItem(*itemManager.myItems[mySelectedIndex]);
			}
			else
			{
				ImGui::Separator();
				ImGui::Spacing();
				ImGui::Text("Add an item to start editing!");

			}
			ImGui::Separator(); 
			ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x * 0.5f) - 50);
			if (ImGui::Button("SAVE", { 100.f, 40.f }))
			{
				SaveItems();
			}
		}
		ImGui::End();
	}
}

void ItemEditor::LoadItems()
{
	nlohmann::json masterJson;
	std::ifstream stream("Assets/Json/Player/Items.json");
	stream >> masterJson;
	stream.close(); 

	ItemManager& itemManager = Main::GetItemManager();

	for (auto& itemJson : masterJson["Items"])
	{
		itemManager.myItems.push_back(DBG_NEW Item);
		Item& item = *itemManager.myItems.back();

		item.myName = itemJson["Name"];
		item.myDescription = itemJson["Description"];
		item.myShortDescription = itemJson["ShortDescription"];
		item.myCooldown = itemJson["Cooldown"];
		if (itemJson.contains("ID"))
		{
			item.myID = itemJson["ID"];
		}
		while (myUsedIDs.find(item.myID) != myUsedIDs.end())
		{
			item.myID = Random::RandomUInt(0U, UINT32_MAX);
		} 
		myUsedIDs.insert(item.myID);

		item.myProcChance = itemJson["ProcChance"];
		if (itemJson.contains("StackProc"))
		{
			item.myStackProcChance = itemJson["StackProc"];
		}
		if (itemJson.contains("Rarity"))
		{
			item.myRarity = eRarity::_from_index(itemJson["Rarity"]);
		}

		std::string path = itemJson["SpritePath"];
		if (path != "")
		{
			item.mySprite = GResourceManager->CreateRef<Engine::TextureResource>(path);
			item.mySprite->RequestLoading();
		}

		for (auto& effectJson : itemJson["Effects"])
		{
			LoadEffect(item, effectJson);
		}
	}
}

void ItemEditor::SaveItems()
{
	nlohmann::json masterJson;

	ItemManager& itemManager = Main::GetItemManager();

	for (auto& item : itemManager.myItems)
	{
		nlohmann::json& itemJson = masterJson["Items"].emplace_back();
		itemJson["Name"] = item->myName;
		itemJson["Description"] = item->myDescription;
		itemJson["ShortDescription"] = item->myShortDescription;
		itemJson["Cooldown"] = item->myCooldown;
		itemJson["ProcChance"] = item->myProcChance;
		itemJson["StackProc"] = item->myStackProcChance;
		itemJson["SpritePath"] = item->mySprite ? item->mySprite.myResource->GetPath() : "";
		itemJson["Rarity"] = item->myRarity._to_index();

		for (auto* effect : item->myEffects)
		{
			itemJson["Effects"].emplace_back();
			effect->Save(itemJson["Effects"].back());
		}
	}
	std::ofstream stream("Assets/Json/Player/Items.json");
	stream << masterJson.dump(3);
	stream.close(); 
}

void ItemEditor::LoadEffect(Item& anItem, nlohmann::json& aJson)
{
	eItemEventType type = eItemEventType::Base;

	if (aJson.contains("Type"))
	{
		type = eItemEventType::_from_index(aJson["Type"]);
	}

	bool shouldLoad = true;
	switch (type)
	{
	case eItemEventType::Base:
		anItem.myEffects.push_back(DBG_NEW ItemEffect);
		break;
	case eItemEventType::VFX:
		anItem.myEffects.push_back(DBG_NEW ItemVFX);
		break;
	default:
		LOG_ERROR(LogType::Items) << "Unsupported item type: item effect wasn't loaded!";
		shouldLoad = false;
		break;
	}

	if (shouldLoad)
	{
		ItemEffectBase* effect = anItem.myEffects.back();
		effect->SetItem(&anItem);
		effect->SetIndex(anItem.myEffects.size() - 1U);
		effect->Load(aJson);
	}
}

void ItemEditor::CreateNewItem()
{
	ItemManager& itemManager = Main::GetItemManager();
	itemManager.myItems.push_back(DBG_NEW Item);
	Item& item = *itemManager.myItems.back(); 

	do
	{
		item.myID = Random::RandomUInt(0U, UINT32_MAX);
	} while (myUsedIDs.find(item.myID) != myUsedIDs.end());

	myUsedIDs.insert(item.myID);
}

void ItemEditor::UpdateItem(Item& anItem)
{
	ImGui::Separator();
	ImGui::Spacing();

	ImGui::InputText("Name", &anItem.myName);
	ImGui::InputTextMultiline("Description", &anItem.myDescription, ImVec2(500.0f, 80.0f));
	ImGui::InputText("Short Description", &anItem.myShortDescription);

	ImGui::Spacing();
	ImGui::Spacing();

	ImGui::InputFloat("Cooldown", &anItem.myCooldown);
	ImGui::InputFloat("Proc Chance", &anItem.myProcChance);
	ImGui::Checkbox("Stack proc chance (linearly)", &anItem.myStackProcChance);
	ImGui::EnumChoice("Rarity", anItem.myRarity);
	
	ImGui::ImageButton(anItem.mySprite ? anItem.mySprite->Get().GetSRV() : nullptr, {50.f, 50.f});
	if (ImGui::BeginDragDropTarget())
	{
		const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(DragDropConstants::Texture.c_str());
		if (payload)
		{
			char path[255] = {};
			memcpy(path, payload->Data, payload->DataSize);

			anItem.mySprite = GResourceManager->CreateRef<Engine::TextureResource>(path);
			anItem.mySprite->RequestLoading();
		}
	}
	ImGui::SameLine();
	ImGui::Text("Sprite");


	ImGui::Separator();
	ImGui::Text("Effects:");
	if (ImGui::Button("+Basic effect"))
	{
		anItem.myEffects.push_back(DBG_NEW ItemEffect);
		anItem.myEffects.back()->SetItem(&anItem);
		anItem.myEffects.back()->SetIndex(anItem.myEffects.size() - 1U);
	}
	ImGui::SameLine();
	if (ImGui::Button("+VFX"))
	{
		anItem.myEffects.push_back(DBG_NEW ItemVFX);
		anItem.myEffects.back()->SetItem(&anItem);
		anItem.myEffects.back()->SetIndex(anItem.myEffects.size() - 1U);
	}

	int effectIndex = 0;
	for (auto* effect : anItem.myEffects)
	{
		if (UpdateEffect(effect, effectIndex))
		{
			std::string deleteText = "Delete##effect" + std::to_string(effectIndex);
			if (ImGui::Button(deleteText.c_str()))
			{
				anItem.myEffects.erase(anItem.myEffects.begin() + effectIndex);
				--effectIndex;
				for (int i = 0; i < anItem.myEffects.size(); ++i)
				{
					anItem.myEffects[i]->SetIndex(i);
				}
			}
		}
		effectIndex++;
	}

}

bool ItemEditor::UpdateEffect(ItemEffectBase* anEffect, int anIndex)
{
	std::string indexText = "##" + std::to_string(anIndex);

	std::string effectString = anEffect->GetType()._to_string();
	if (ImGui::CollapsingHeader(std::string(effectString + indexText).c_str()))
	{
		anEffect->Expose(anIndex);
		return true;
	}
	return false;
}
