#include "pch.h"
#include "ItemVFX.h"
#include "Engine/Utils/TimerManager.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/GameObject/Components/VFXComponent.h"
#include "Engine/Editor/DragDropConstants.h"
#include "Engine/VFX/VFXUtilities.h"
#include "Engine/GameObject/Prefab/GameObjectPrefab.h"
#include "Engine/ResourceManagement/Resources/VFXResource.h"

const eItemEventType ItemVFX::GetType()
{
	return eItemEventType::VFX;
}

void ItemVFX::Save(nlohmann::json& aJson)
{
	ItemEffectBase::Save(aJson);
	aJson["Delay"] = myDelay;
	aJson["Path"] = myVFXPath;
	aJson["Duration"] = myDuration;
	aJson["Offset"]["x"] = myPositionOffset.x;
	aJson["Offset"]["y"] = myPositionOffset.y;
	aJson["Offset"]["z"] = myPositionOffset.z;
	aJson["Follow"] = myShouldFollowTarget;
	aJson["UsePrefab"] = myUsePrefab;
}

void ItemVFX::Load(nlohmann::json& aJson)
{
	ItemEffectBase::Load(aJson);
	if (aJson.contains("Delay"))
	{
		myDelay = aJson["Delay"];
	}
	if (aJson.contains("Duration"))
	{
		myDuration = aJson["Duration"];
	}
	if (aJson.contains("Path"))
	{
		myVFXPath = aJson["Path"];
	}
	if (aJson.contains("Offset"))
	{
		myPositionOffset.x = aJson["Offset"]["x"];
		myPositionOffset.y = aJson["Offset"]["y"];
		myPositionOffset.z = aJson["Offset"]["z"];
	}
	if (aJson.contains("Follow"))
	{
		myShouldFollowTarget = aJson["Follow"];
	}
	if (aJson.contains("UsePrefab"))
	{
		myUsePrefab = aJson["UsePrefab"];
	}
	if (!myUsePrefab)
	{
		myVFX = GResourceManager->CreateRef<Engine::VFXResource>(myVFXPath);
		myVFX->RequestLoading();
	}
}

void ItemVFX::Expose(int anIndex)
{
	ItemEffectBase::Expose(anIndex); 
	const std::string indexText = "##" + std::to_string(anIndex);

	if (ImGui::Checkbox("Use Prefab", &myUsePrefab))
	{
		myVFXPath = "";
	}

	const std::string path = myVFXPath + indexText;
	ImGui::Button(path.c_str(), ImVec2(ImGui::GetContentRegionAvail().x * 0.7f, 25));
	if (ImGui::BeginDragDropTarget())
	{
		if (myUsePrefab)
		{
			if (auto* payload = ImGui::AcceptDragDropPayload(DragDropConstants::GameObjectPrefab.c_str()))
			{
				char path[255] = {};
				memcpy(path, payload->Data, payload->DataSize);
				myVFXPath = path;
			}
		}
		else
		{
			if (auto* payload = ImGui::AcceptDragDropPayload(DragDropConstants::VFX.c_str()))
			{
				char path[255] = {};
				memcpy(path, payload->Data, payload->DataSize);
				myVFXPath = path;
			}
		}


	}
	ImGui::SameLine();
	if (myUsePrefab)
	{
		ImGui::Text("Prefab");
	}
	else
	{
		ImGui::Text("VFX");
	}


	ImGui::InputFloat3("Offset Position", &myPositionOffset.x);
	ImGui::Checkbox("Follow Target", &myShouldFollowTarget);

	ImGui::InputFloat(("Delay" + indexText).c_str(), &myDelay);
	ImGui::InputFloat(("Duration" + indexText).c_str(), &myDuration);

}

void ItemVFX::ApplyEffect(GameObject& anObject, const ItemEventData* someData)
{
	if (myVFXPath != "")
	{

		CU::Vector3f hitPos = someData->hitPos;
		if (anObject.GetLayer() & eLayer::PLAYER)
		{
			hitPos = anObject.GetTransform().GetPosition();
		}
		if (myUsePrefab)
		{
			Weak<GameObject> weakGO = anObject.GetWeak();
			if (myDelay > 0.f)
			{
				Main::GetTimerManager().SetTimer(myDelay, [this, weakGO, hitPos] {SpawnPrefab(weakGO, hitPos); });
			}
			else
			{
				SpawnPrefab(weakGO, hitPos);
			}
		}
		else
		{
			Weak<GameObject> weakGO = anObject.GetWeak();
			if (myDelay > 0.f)
			{
				Main::GetTimerManager().SetTimer(myDelay, [this, weakGO, hitPos] {PlayVFX(weakGO, hitPos); });
			}
			else
			{
				PlayVFX(weakGO, hitPos);
			}
		}
	}
}

void ItemVFX::PlayVFX(Weak<GameObject> anObject, const CU::Vector3f& aPosition)
{
	if (Shared<GameObject> shared = anObject.lock())
	{
		GameObject* object = shared->GetScene()->AddGameObject<GameObject>();
		if (myShouldFollowTarget)
		{
			object->GetTransform().SetParent(&shared->GetTransform());
		}
		CU::Vector3f offsetPos =
			myPositionOffset.x * shared->GetTransform().Right() +
			myPositionOffset.y * shared->GetTransform().Up() +
			myPositionOffset.z * shared->GetTransform().Forward();
		object->GetTransform().SetPosition(aPosition + offsetPos);

		Engine::VFXComponent* comp = object->AddComponent<Engine::VFXComponent>(myVFX->Get());
		comp->Play();
		comp->SetDuration(myDuration);
		comp->AutoDestroy();
	}
}

void ItemVFX::SpawnPrefab(Weak<GameObject> anObject, const CU::Vector3f& aPosition)
{
	if (Shared<GameObject> shared = anObject.lock())
	{
		GameObjectPrefabRef prefabRef = GResourceManager->CreateRef<Engine::GameObjectPrefabResource>(myVFXPath);
		prefabRef->Load();
		GameObject& prefab = Engine::GameObjectPrefab::InstantiatePrefabInstance(prefabRef, *shared->GetScene());
		
		if (myShouldFollowTarget)
		{
			prefab.GetTransform().SetParent(&shared->GetTransform());
		}
		CU::Vector3f offsetPos =
			myPositionOffset.x * shared->GetTransform().Right() +
			myPositionOffset.y * shared->GetTransform().Up() +
			myPositionOffset.z * shared->GetTransform().Forward();
		prefab.GetTransform().SetPosition(aPosition + offsetPos);

		Weak<GameObject> weak = prefab.GetWeak();

		if (myDuration > 0.f)
		{
			Main::GetTimerManager().SetTimer(myDuration, 
				[weak]
				{
					if (Shared<GameObject> shared = weak.lock())
					{
						shared->Destroy();
					}
				}
			);
		}
	}
}
