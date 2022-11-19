#include "pch.h"
#include "ItemEffect.h"
#include "Engine/GameObject/GameObject.h"
#include "Components/HealthComponent.h"
#include "Engine/Utils/TimerManager.h"
#include "Items/Buff.h"
#include "Components/Item/BouncingDamageComponent.h"
#include "Engine/GameObject/Components/ModelComponent.h"
#include "Engine/Editor/ImGuiHelper.h"
#include "Player/Player.h"
#include <Prototype/RorPlayer.h>
#include <Components/MoveComponent.h>
#include <AI/PollingStation/PollingStationComponent.h>
#include <Components/ExplosiveObjectComponent.h>
#include "Engine/ResourceManagement/Resources/GameObjectPrefabResource.h"
#include "Engine/GameObject/Prefab/GameObjectPrefab.h"
#include <Prototype/HomingProjectile.h>
#include "Item.h"

#include "Engine\GameObject\Components\AnimatorComponent.h"
#include "Engine\GameObject\Components\AudioComponent.h"

ItemEffect::ItemEffect() :
	myEffectType(eItemEffect::Heal)
{
}

ItemEffect::~ItemEffect()
{
	if (Main::Exists())
	{
		Main::GetTimerManager().StopTimer(myTimer);
	}
}

const eItemEventType ItemEffect::GetType()
{
	return eItemEventType::Base;
}

void ItemEffect::Save(nlohmann::json& aJson)
{
	ItemEffectBase::Save(aJson);
	aJson["Effect"] = myEffectType._to_index();
	aJson["Duration"] = myDuration;
	aJson["Strength"] = myStrength;
	aJson["StackStrength"] = myStackStrength;
	aJson["StackApply"] = myStackAppliedEffect;
	aJson["MaxStacks"] = myMaxStacks;

	aJson["CustomFloat1"] = myCustomFloat1;
	aJson["CustomFloat2"] = myCustomFloat2;
	aJson["CustomFloat3"] = myCustomFloat3;
	aJson["CustomFloat4"] = myCustomFloat4;
	aJson["CustomFloat5"] = myCustomFloat5;
	aJson["CustomInt1"] = myCustomInt1;
}

void ItemEffect::Load(nlohmann::json& aJson)
{
	ItemEffectBase::Load(aJson);
	myEffectType = eItemEffect::_from_index(aJson["Effect"]);
	myDuration = aJson["Duration"];
	myStrength = aJson["Strength"];
	if (aJson.contains("StackStrength"))
	{
		myStackStrength = aJson["StackStrength"];
	}
	if (aJson.contains("StackApply"))
	{
		myStackAppliedEffect = aJson["StackApply"];
		myMaxStacks = aJson["MaxStacks"];
	}

	if (aJson.contains("Custom1"))
	{
		myCustomFloat1 = aJson["Custom1"];
	}
	if (aJson.contains("CustomFloat1"))
	{
		myCustomFloat1 = aJson["CustomFloat1"];
	}
	if (aJson.contains("CustomFloat2"))
	{
		myCustomFloat2 = aJson["CustomFloat2"];
	}
	if (aJson.contains("CustomFloat3"))
	{
		myCustomFloat3 = aJson["CustomFloat3"];
	}
	if (aJson.contains("CustomFloat4"))
	{
		myCustomFloat4 = aJson["CustomFloat4"];
	}
	if (aJson.contains("CustomFloat5"))
	{
		myCustomFloat5 = aJson["CustomFloat5"];
	}
	if (aJson.contains("CustomInt1"))
	{
		myCustomInt1 = aJson["CustomInt1"];
	}
}

void ItemEffect::Expose(int anIndex)
{
	ItemEffectBase::Expose(anIndex);

	const std::string indexText = "##" + std::to_string(anIndex);
	ImGui::EnumChoice(("Effect" + indexText).c_str(), myEffectType);
	ImGuiContextualEffects(anIndex);

	ImGui::InputFloat(("Duration" + indexText).c_str(), &myDuration);
	ImGui::InputFloat(("Strength" + indexText).c_str(), &myStrength);
	ImGui::Checkbox("Stack Strength", &myStackStrength);
	ImGui::Checkbox("Refresh debuff/buff", &myStackAppliedEffect);
	if (myStackAppliedEffect)
	{
		ImGui::InputInt("Max buff stacks", &myMaxStacks);
	}
}

void ItemEffect::ImGuiContextualEffects(int anIndex)
{
	switch (myEffectType._value)
	{
	case eItemEffect::DragTowardPlayer:
	case eItemEffect::DragTowardTarget:
	case eItemEffect::Knockback:
	case eItemEffect::Stun:
	{
		std::string warningText = "Effect \"";
		warningText.append(myEffectType._to_string());
		warningText += "\" is currently unsupported.";
		ImGui::TextColored(ImVec4(1.f, 0.f, 0.f, 1.f), warningText.c_str());
		break;
	}
	case eItemEffect::SpawnChainLightning:
		ImGui::InputFloat("Bounce Radius", &myCustomFloat1);
		ImGui::InputFloat("Chain Delay", &myCustomFloat2);
		ImGui::InputFloat("Lightning VFX Duration", &myCustomFloat3);
		ImGui::InputInt("Bounces", &myCustomInt1);
		break;
	case eItemEffect::DamageOverTime:
		if (myCustomFloat1 <= 0.00001f)
		{
			ImGui::TextColored({ 1.f, 0.f, 0.f, 1.f }, "Tick time must be larger than 0.00001");
		}
		ImGui::InputFloat(("Tick time##" + std::to_string(anIndex)).c_str(), &myCustomFloat1);
		break;
	case eItemEffect::Homing:
		ImGui::InputFloat("Movement Speed", &myCustomFloat1);
		ImGui::InputFloat("Life Range", &myCustomFloat2);
		ImGui::InputFloat("Rotation Speed", &myCustomFloat3);
		ImGui::InputFloat("Seek Radius", &myCustomFloat4);
		break;
	case eItemEffect::Pull:
		ImGui::InputFloat("Speed Multiplier", &myCustomFloat1);
		ImGui::InputFloat("Delay (Sec)", &myCustomFloat2);
		ImGui::InputFloat("LifeTime (Sec) after Delay", &myCustomFloat3);
		ImGui::InputFloat("Elasticity (Pref. less than Lifetime)", &myCustomFloat4);
		ImGui::InputFloat("Radius (Set same as EnemyAOE)", &myCustomFloat5);
	}
}

bool ItemEffect::Stacking()
{
	return myStackStrength;	//If more stack types are added later: check these as well
}

void ItemEffect::ApplyEffect(GameObject& anObject, const ItemEventData* someData)
{
	HealthComponent* healthComp = anObject.GetComponent<HealthComponent>();
	if (!healthComp)
	{
		LOG_ERROR(LogType::Items) << "ITEM EFFECT CANT FIND HEALTHCOMPONENT";
		return;
	}

	bool applyEffect = true;
	if (myDuration > 0.f && myStackAppliedEffect)
	{
		if (Buff* buff = healthComp->GetBuff(BuffID(myItem->GetID(), myIndex)))
		{
			applyEffect = buff->GetStacks() < myMaxStacks;
		}

	}
	if (myEffectType == +eItemEffect::DamageOverTime)
	{
		applyEffect = true;			//Bad solution, but oh well
	}

	bool applyBuff = true;
	if (applyEffect)
	{
		switch (myEffectType)
		{
		case eItemEffect::Damage:
			healthComp->ApplyDamage(myStrength);
			break;
		case eItemEffect::DamagePercentage:
			healthComp->ApplyDamage(someData->damage * myStrength);
			break;
		case eItemEffect::Heal:
			healthComp->ApplyHealing(myStrength);
			break;
		case eItemEffect::Knockback:
		{
			CU::Vector3f direction = someData->direction;
			direction.Normalize();
			healthComp->ApplyKnockback(direction);
			break;
		}
		case eItemEffect::Stun:
			healthComp->ApplyStun(myDuration);
			break;
		case eItemEffect::DamageOverTime:
			ApplyDoT(*healthComp);
			applyBuff = false;
			break;
		case eItemEffect::Invulnerability:
			healthComp->SetIsInvincible(true);
			break;
		case eItemEffect::StatDamage:
		{
			PlayerStats& stats = anObject.GetComponent<Player>()->GetStats();
			stats.SetBaseDamage(stats.GetBaseDamage() + myStrength);
			break;
		}
		case eItemEffect::StatDamageModifier:
		{
			PlayerStats& stats = anObject.GetComponent<Player>()->GetStats();
			stats.SetDamageMod(stats.GetDamageMod() + myStrength);
			break;
		}
		case eItemEffect::StatAttackSpeed:
		{
			PlayerStats& stats = anObject.GetComponent<Player>()->GetStats();
			stats.SetAttackSpeed(stats.GetAttackSpeed() + myStrength);
			break;
		}
		case eItemEffect::StatJumpForce:
		{
			PlayerStats& stats = anObject.GetComponent<Player>()->GetStats();
			stats.SetJumpForce(stats.GetJumpForce() + myStrength);
			break;
		}
		case eItemEffect::StatShield:
			healthComp->AddPermanentShield(myStrength);
			break;
		case eItemEffect::Barrier:
			healthComp->AddBarrier(myStrength);
			break;
		case eItemEffect::SpawnChainLightning:
			SpawnChainLightning(anObject);
			break;
		case eItemEffect::StatRecievedDamageMod:
			healthComp->SetDamageModifier(healthComp->GetDamageModifier() + myStrength);
			break;
		case eItemEffect::Homing:
		{
			const GameObjectPrefabRef& homingProjPrefab = Main::GetPlayer()->GetComponent<Player>()->GetHomingprojectilePrefab();
			if (homingProjPrefab && homingProjPrefab->IsValid())
			{
				GameObject* tempGO = &homingProjPrefab->Get().Instantiate(*anObject.GetScene());
				tempGO->GetTransform().SetPosition(anObject.GetTransform().GetPosition());
				tempGO->SetName("HOMING");
				HomingProjectile* homingProj = tempGO->GetComponent<HomingProjectile>();
				if (Main::GetPlayer())
				{
					if (Player* player = Main::GetPlayer()->GetComponent<Player>())
					{
						const float damage = player->GetStats().GetDamage();
						homingProj->Setup(anObject.GetTransform().Forward(), /*Movement Speed*/myCustomFloat1, /*Damage*/ damage, /*LifeRange*/myCustomFloat2, /*RotationSpeed*/myCustomFloat3, /*SeekRadius*/myCustomFloat4);
					}

				}

			}
			else
			{
				LOG_ERROR(LogType::Game) << "HOMING PROJECTILE PREFAB WAS NOT VALID";
			}
			break;
		}
		case eItemEffect::StatMaxHealth:
			healthComp->AddMaxHealth(myStrength);
			break;
		case eItemEffect::StatMaxHealthPercent:
			healthComp->AddMaxHealthPercent(myStrength);
			break;
		case eItemEffect::Pull:
		{
			if (!(anObject.GetLayer() & eLayer::IMMOVABLE))
			{
				MoveComponent* tempMover = anObject.AddComponent<MoveComponent>(/*Damage*/myStrength, /*Speed*/myCustomFloat1, /*Delay*/myCustomFloat2, /*LifeTime*/myCustomFloat3, /*Acceleration*/myCustomFloat4, /*Radius*/myCustomFloat5);
				tempMover->Init(someData->hitPos);

				auto gO = anObject.GetScene()->AddGameObject<GameObject>();
				auto audio = gO->AddComponent<Engine::AudioComponent>();

				audio->AddEvent("BlakcHole", "event:/SFX/PLR/Pickups/BlackHole");
				audio->PlayEvent("BlakcHole");
			}
		}
		break;
		case eItemEffect::JumpAmountChange:
		{
			if (auto* player = Main::GetPlayer())
			{
				player->GetComponent<Player>()->GetStats().AddJumpAmount(myStrength);
			}

			break;
		}
		case eItemEffect::CooldownMultiplierChange:
		{
			if (auto* player = Main::GetPlayer())
			{
				player->GetComponent<Player>()->GetStats().AddCooldownMultiplier(-myStrength);
			}

			break;
		}
		case eItemEffect::Explosion:
		{
			GameObject* tempGO = &Main::GetPlayer()->
				GetComponent<Player>()->
				GetExplosionPrefab()->
				Get().
				Instantiate(*anObject.GetScene());
			tempGO->GetTransform().SetPosition(someData->hitPos);

			break;
		}

		case eItemEffect::Evasion:
			Main::GetPlayer()->GetComponent<HealthComponent>()->AddDodgeChance(myStrength);
			break;
		case eItemEffect::StatCritChance:
		{
			PlayerStats& stats = anObject.GetComponent<Player>()->GetStats();
			stats.SetCritChance(stats.GetCritChance() + myStrength);
			break;
		}
		case eItemEffect::StatCritDamageMod:
		{
			PlayerStats& stats = anObject.GetComponent<Player>()->GetStats();
			stats.SetCritDamageMultiplier(stats.GetCritDamageMultiplier() + myStrength);
			break;
		}
		case eItemEffect::StatMoveSpeed:
		{
			Player* player = anObject.GetComponent<Player>();
			PlayerStats& stats = player->GetStats();

			float movementSpeed = stats.GetMovementSpeed() + myStrength;
			stats.SetMovementSpeed(movementSpeed);

			if (stats.GetRunAnimationSpeedUpCount() < 4)
			{
				player->GetAnimatorComponent().GetController().SetLayerTimeScale("BaseLayer", movementSpeed / player->GetSettings().animation.movementStartSpeed);
				stats.IncrementRunAnimationSpeedUpCount();
			}

			break;
		}

		default:
		{
			std::string warningDesc("Item effect \"" + std::string(myEffectType._to_string()) + "\" is currently unsupported.");
			LOG_WARNING(LogType::Items) << warningDesc;
			break;
		}
		}
	}
	if (applyBuff)
	{
		ApplyBuff(healthComp);
	}

}

void ItemEffect::ApplyBuff(HealthComponent* aHealthComp)
{
	if (myDuration > 0.f)
	{
		aHealthComp->ApplyBuff(CreateEndBuff(*aHealthComp->GetGameObject()), myStackAppliedEffect ? myMaxStacks : 1);
	}
}

std::unique_ptr<Buff> ItemEffect::CreateEndBuff(GameObject& anObject)
{
	std::unique_ptr<Buff> buff = std::make_unique<Buff>(GetEndEffect(), anObject, BuffID(myItem->GetID(), myIndex), myDuration);
	return buff;
}

void ItemEffect::SpawnChainLightning(GameObject& anObject)
{
	GameObjectPrefabRef prefab = GResourceManager->CreateRef<Engine::GameObjectPrefabResource>("Assets/Prefabs/BouncingLightning.prefabg");
	prefab->Load();
	GameObject& newObject = Engine::GameObjectPrefab::InstantiatePrefabInstance(prefab, *anObject.GetScene());

	newObject.GetTransform().SetPosition(anObject.GetTransform().GetPosition());

	BouncingDamageComponent* newComp = newObject.GetComponent<BouncingDamageComponent>();
	newComp->Init(&anObject.GetTransform(), myCustomInt1, myCustomFloat1, myStrength, myCustomFloat2, myCustomFloat3);
}

void ItemEffect::ApplyDoT(HealthComponent& aHealthComp)
{
	const float tickTime = myCustomFloat1;
	const float tickDamage = myStrength * tickTime;
	std::function<void(GameObject*, float)> dotEffect = [tickDamage](GameObject* anObject, float someStacks)
	{
		anObject->GetComponent<HealthComponent>()->ApplyDamage(tickDamage * someStacks, false, nullptr, eDamageType::Bleed);
	};
	aHealthComp.ApplyBuff(std::make_unique<Buff>(dotEffect, *aHealthComp.GetGameObject(), BuffID(myItem->GetID(), myIndex), myDuration, tickTime), myStackAppliedEffect ? myMaxStacks : 1);
}

std::function<void(GameObject*, float)> ItemEffect::GetEndEffect()
{
	switch (myEffectType)
	{

	case eItemEffect::Damage:
		return [&](GameObject* anObject, float someStacks)
		{
			anObject->GetComponent<HealthComponent>()->ApplyHealing(myStrength * someStacks);
		};

	case eItemEffect::Heal:
		return [&](GameObject* anObject, float someStacks)
		{
			anObject->GetComponent<HealthComponent>()->ApplyHealing(-myStrength * someStacks);
		};

	case eItemEffect::Invulnerability:
		return [](GameObject* anObject, float /*someStacks*/)
		{
			anObject->GetComponent<HealthComponent>()->SetIsInvincible(false);
		};
	case eItemEffect::StatDamage:
		return [&](GameObject* anObject, float someStacks)
		{
			PlayerStats& stats = anObject->GetComponent<Player>()->GetStats();
			stats.SetBaseDamage(stats.GetBaseDamage() - myStrength * someStacks);
		};
	case eItemEffect::StatDamageModifier:
		return [&](GameObject* anObject, float someStacks)
		{
			PlayerStats& stats = anObject->GetComponent<Player>()->GetStats();
			stats.SetDamageMod(stats.GetDamageMod() - myStrength * someStacks);
		};
	case eItemEffect::StatAttackSpeed:
		return [&](GameObject* anObject, float someStacks)
		{
			PlayerStats& stats = anObject->GetComponent<Player>()->GetStats();
			stats.SetAttackSpeed(stats.GetAttackSpeed() - myStrength * someStacks);
		};
	case eItemEffect::StatJumpForce:
		return [&](GameObject* anObject, float someStacks)
		{
			PlayerStats& stats = anObject->GetComponent<Player>()->GetStats();
			stats.SetJumpForce(stats.GetJumpForce() - myStrength * someStacks);
		};
	case eItemEffect::StatShield:
		return [&](GameObject* anObject, float someStacks)
		{
			anObject->GetComponent<HealthComponent>()->AddPermanentShield(-myStrength * someStacks);
		};
	case eItemEffect::Barrier:
		return [&](GameObject* anObject, float someStacks)
		{
			anObject->GetComponent<HealthComponent>()->RemoveBarrier(myStrength * someStacks);
		};
	case eItemEffect::StatRecievedDamageMod:
		return [&](GameObject* anObject, float someStacks) {
			HealthComponent* hpComp = anObject->GetComponent<HealthComponent>();
			hpComp->SetDamageModifier(hpComp->GetDamageModifier() - myStrength * someStacks);
		};
	case eItemEffect::StatMaxHealth:
		return [&](GameObject* anObject, float someStacks)
		{
			anObject->GetComponent<HealthComponent>()->AddMaxHealth(-myStrength * someStacks);
		};
	case eItemEffect::StatMaxHealthPercent:
		return [&](GameObject* anObject, float someStacks)
		{
			anObject->GetComponent<HealthComponent>()->AddMaxHealthPercent(-myStrength * someStacks);
		};
	case eItemEffect::StatCritChance:
	{
		return[&](GameObject* anObject, float someStacks)
		{
			PlayerStats& stats = anObject->GetComponent<Player>()->GetStats();
			stats.SetCritChance(stats.GetCritChance() - myStrength * someStacks);
		};
	}
	case eItemEffect::StatCritDamageMod:
	{
		return[&](GameObject* anObject, float someStacks)
		{
			PlayerStats& stats = anObject->GetComponent<Player>()->GetStats();
			stats.SetCritDamageMultiplier(stats.GetCritDamageMultiplier() - myStrength * someStacks);
		};
	}
	case eItemEffect::StatMoveSpeed:
	{
		return[&](GameObject* anObject, float someStacks)
		{
			Player* player = anObject->GetComponent<Player>();
			PlayerStats& stats = player->GetStats();

			float movementSpeed = stats.GetMovementSpeed() - myStrength * someStacks;
			stats.SetMovementSpeed(movementSpeed);

			if (stats.GetRunAnimationSpeedUpCount() < 4)
			{
				player->GetAnimatorComponent().GetController().SetLayerTimeScale("BaseLayer", movementSpeed / player->GetSettings().animation.movementStartSpeed);
				stats.IncrementRunAnimationSpeedUpCount();
			}
		};
	}
	case eItemEffect::CooldownMultiplierChange:
	{
		return[&](GameObject* anObject, float someStacks)
		{
			PlayerStats& stats = anObject->GetComponent<Player>()->GetStats();
			stats.AddCooldownMultiplier(myStrength * someStacks);
		};
	}
	case eItemEffect::Evasion:
	{
		return [&](GameObject* anObject, float someStacks)
		{
			Main::GetPlayer()->GetComponent<HealthComponent>()->AddDodgeChance(-myStrength * someStacks);
		};
	}
	case eItemEffect::JumpAmountChange:
	{
		return [&](GameObject* anObject, float someStacks)
		{
			if (auto* player = Main::GetPlayer())
			{
				player->GetComponent<Player>()->GetStats().AddJumpAmount(-myStrength * someStacks);
			}
		};
	}
	default:
	{
		return nullptr;
	}
	}
}