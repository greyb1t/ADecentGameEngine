#include "pch.h"
#include "ItemEffectBase.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/Editor/ImGuiHelper.h"

ItemEffectBase::ItemEffectBase() : 
	myTriggeringEvent(eItemEvent::OnPickup), myTarget(eItemTarget::Player)
{
}

bool ItemEffectBase::ResolveEffect(const ItemEventData* someData, bool aHasProcced)
{
	if (myShouldProc && !aHasProcced)
	{
		return false;
	}

	switch (myTarget)
	{
	case eItemTarget::Player:
		ApplyEffect(*Main::GetPlayer(), someData);
		break;
	case eItemTarget::Enemy:
		if (someData && someData->targetObject)
		{
			ApplyEffect(*someData->targetObject, someData);
		}
		else
		{
			LOG_ERROR(LogType::Items) << "Target type was enemy, but no enemy object was provided! Event was: " << myTriggeringEvent._to_string();
		}

		break;
	case eItemTarget::PlayerAOE:
	{
		std::vector<OverlapHit> hits;
		if (Main::GetPlayer()->GetScene()->SphereCastAll(Main::GetPlayer()->GetTransform().GetPosition(), myAOE, eLayer::ENEMY, hits))
		{
			for (auto& hit : hits)
			{
				ApplyEffect(*hit.GameObject, someData);
			}
		}
	}
	break;
	case eItemTarget::TargetAOE:
	{
		if (someData)
		{
			std::vector<OverlapHit> hits;
			if (someData->targetObject->GetScene()->SphereCastAll(someData->hitPos, myAOE, eLayer::ENEMY, hits))
			{
				for (auto& hit : hits)
				{
					ApplyEffect(*hit.GameObject, someData);
				}
			}
		}
		else
		{
			LOG_ERROR(LogType::Items) << "Tried to get the target enemy, but it was not provided! Event was: " << myTriggeringEvent._to_string();
		}
		break;
	}
	case eItemTarget::EnemyExclusiveAOE:
		if (someData)
		{
			std::vector<OverlapHit> hits;
			if (someData->targetObject->GetScene()->SphereCastAll(someData->hitPos, myAOE, eLayer::ENEMY, hits))
			{
				auto isTarget = [&someData](const OverlapHit& aHit) ->bool {return aHit.GameObject->GetUUID() == someData->targetObject->GetUUID(); };
				auto eraseIterator = std::find_if(hits.begin(), hits.end(), isTarget);
				if (eraseIterator != hits.end())
				{
					hits.erase(eraseIterator);
				}
				for (auto& hit : hits)
				{
					ApplyEffect(*hit.GameObject, someData);
				}
			}
		}
		else
		{
			LOG_ERROR(LogType::Items) << "Tried to get the target enemy, but it was not provided! Event was: " << myTriggeringEvent._to_string();
		}
		break;
	default:
		LOG_ERROR(LogType::Items) << "Unsupported item target: \"" << myTarget._to_string() << "\"";
	}

	return myTriggersCooldown;
}

const eItemEvent& ItemEffectBase::GetEvent() const
{
	return myTriggeringEvent;
}

void ItemEffectBase::Save(nlohmann::json& aJson)
{
	aJson["Event"] = myTriggeringEvent._to_index();
	aJson["Target"] = myTarget._to_index();
	aJson["AOE"] = myAOE;
	aJson["Proc"] = myShouldProc;
	aJson["UseCooldown"] = myTriggersCooldown;

	aJson["Type"] = GetType()._to_index();
}

void ItemEffectBase::Load(nlohmann::json& aJson)
{
	myTriggeringEvent = eItemEvent::_from_index(aJson["Event"]);
	myTarget = eItemTarget::_from_index(aJson["Target"]);

	myAOE = aJson["AOE"];

	myShouldProc = aJson["Proc"];
	myTriggersCooldown = aJson["UseCooldown"];
}

void ItemEffectBase::Expose(int anIndex)
{
	const std::string indexText = "##" + std::to_string(anIndex);
	ImGui::EnumChoice(("Event" + indexText), myTriggeringEvent);
	ImGui::EnumChoice(("Target" + indexText), myTarget);
	if (myTarget == +eItemTarget::PlayerAOE || myTarget == +eItemTarget::TargetAOE)
	{
		ImGui::SetCursorPosX(30);
		ImGui::InputFloat(("Radius" + indexText).c_str(), &myAOE);
	}
	ImGui::Checkbox(("Use Proc" + indexText).c_str(), &myShouldProc);
	ImGui::Checkbox(("Trigger Cooldown" + indexText).c_str(), &myTriggersCooldown);
}

void ItemEffectBase::SetItem(Item* anItem)
{
	myItem = anItem; 
}

void ItemEffectBase::SetIndex(uint32_t anIndex)
{
	myIndex = anIndex; 
}
