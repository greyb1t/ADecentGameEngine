#include "pch.h"
#include "SpikeAttackState.h"

#include "../Objects/BossSpikeExtension.h"
#include "Game/Player/Player.h"

LevelBossStates::SpikeAttackState::SpikeAttackState(LevelBoss& aBoss, SpikeAttackVars& someVariables) 
	: EngageState(aBoss), myVars(someVariables)
{
	ExitAttack();

	const int size = myVars.myBossSpikes.size();
	for (int i = 0; i < size; i++)
	{
		if (auto animator = myVars.myBossSpikes[i].Get()->GetComponent<Engine::AnimatorComponent>())
		{
			myAnimators.push_back(animator);

			auto& controller = myAnimators[i]->GetController();
			controller.SetInt("RandomIdle", Random::RandomInt(0, 2));
		}
	}

	myVars.myFollowTargetProgress = 0.0f;
	myVars.myLingeringAtTargetProgress = 0.0f;

	myAnimState = AnimState::None;
}

void LevelBossStates::SpikeAttackState::ForceUpdate()
{
	myVars.myOverrideProgress -= Time::DeltaTime;
	if (myAnimState == AnimState::Opening)
	{
		if (myVars.myOverrideProgress <= 0.0f)
		{
			myVars.myOverrideProgress = 0.0f;
			myAnimState = AnimState::None;
		}
	}
	else if (myAnimState == AnimState::Closing)
	{
		if (myVars.myOverrideProgress <= 0.0f)
		{
			myVars.myOverrideProgress = 0.0f;
			myAnimState = AnimState::None;
		}
	}

	if (!myBoss.IsAbilityUnlocked(LevelBossState::SpikeAttack))
		return;

	if (myVars.myCooldownProgress > 0.0f)
	{
		if (!myBoss.IsDisengaged() && !myBoss.IsDead())
		{
			myVars.myCooldownProgress -= Time::DeltaTime;
			if (myVars.myCooldownProgress <= 0.0f)
			{
				EnterAttack();
				myVars.myCooldownProgress = 0.0f;
			}
		}
	}

	if (myVars.myFollowTargetProgress > 0.0f)
	{
		myVars.myFollowTargetProgress -= Time::DeltaTime;

		auto pollingStation = GetPollingStation();
		Attack(pollingStation->GetPlayerPos());

		if (myVars.myFollowTargetProgress <= 0.0f)
		{
			myVars.myFollowTargetProgress = 0.0f;
			myVars.myLingeringAtTargetProgress = myVars.myLingeringAtTargetDuration;
		}
	}

	if (myVars.myLingeringAtTargetProgress > 0.0f)
	{
		myVars.myLingeringAtTargetProgress -= Time::DeltaTime;

		if (myVars.myLingeringAtTargetProgress <= 0.0f)
		{
			ExitAttack();
			myVars.myLingeringAtTargetProgress = 0.0f;
		}
	}

	SpikeAttackSequence();
}

void LevelBossStates::SpikeAttackState::OnEnableAttack()
{
	myAnimState = AnimState::Opening;
	myVars.myOverrideProgress = myVars.myOverrideDuration;

	for (int i = 0; i < myAnimators.size(); i++)
	{
		auto& controller = myAnimators[i]->GetController();
		controller.SetBool("HasUnlockedSpikes", true);
	}
}

void LevelBossStates::SpikeAttackState::OnDisableAttack()
{
	myAnimState = AnimState::Closing;
	myVars.myOverrideProgress = myVars.myOverrideDuration;

	for (int i = 0; i < myAnimators.size(); i++)
	{
		auto& controller = myAnimators[i]->GetController();
		controller.SetBool("HasUnlockedSpikes", false);
	}
}

void LevelBossStates::SpikeAttackState::Attack(const Vec3f& aPlayerPosition)
{
	myVars.myTimeBetweenSpikesProgress -= Time::DeltaTime;
	
	if (myVars.myTimeBetweenSpikesProgress <= 0.0f)
	{
		RayCastHit hit;
		auto scene = myBoss.GetGameObject()->GetScene();
		if (scene->RayCast(GetPredictedPosition(), Vec3f(0.0f, -1.0f, 0.0f), 1000.0f, eLayer::DEFAULT | eLayer::GROUND, hit))
		{
			Spike spike;
			spike.myPosition = hit.Position;
			spike.myTelegraphProgress = spike.myTelegraphDuration = myVars.mySpikeTelegraphingDuration;
			mySpikes.push_back(spike);
		}
		myVars.myTimeBetweenSpikesProgress = myVars.myTimeBetweenSpikesDuration;
	}
}

void LevelBossStates::SpikeAttackState::EnterAttack()
{
	myVars.myFollowTargetProgress = myVars.myFollowTargetDuration;

	for (int i = 0; i < myAnimators.size(); i++)
	{
		if (myAnimators[i]->HasMachine())
		{
			auto& controller = myAnimators[i]->GetController();
			controller.Trigger("TriggerSpikes");
			controller.SetBool("UsingSpikes", true);
		}
	}
}

void LevelBossStates::SpikeAttackState::ExitAttack()
{
	myVars.myCooldownDuration = Random::RandomFloat(myVars.myCooldownDurationMin, myVars.myCooldownDurationMax);
	myVars.myCooldownProgress = myVars.myCooldownDuration;

	for (int i = 0; i < myAnimators.size(); i++)
	{
		if (myAnimators[i]->HasMachine())
		{
			auto& controller = myAnimators[i]->GetController();
			controller.SetBool("UsingSpikes", false);
			controller.SetInt("RandomIdle", Random::RandomInt(0, 2));
		}
	}
}

void LevelBossStates::SpikeAttackState::SpikeAttackSequence()
{
	for (int i = mySpikes.size() - 1; i >= 0; i--)
	{
		if (myBoss.IsDead())
		{
			mySpikes.clear();
			break;
		}

		Spike& currentSpike = mySpikes[i];

		if (currentSpike.myTelegraphProgress > 0.0f)
		{
			currentSpike.myTelegraphProgress -= Time::DeltaTime;

			const float ratio = currentSpike.myTelegraphProgress / currentSpike.myTelegraphDuration;

			const float radius = myVars.mySpikeRadius;
			GDebugDrawer->DrawSphere3D(DebugDrawFlags::Gameplay, currentSpike.myPosition, radius, 0.0f, Vec4f(1, 1, 0, 1));
			GDebugDrawer->DrawSphere3D(DebugDrawFlags::Gameplay, currentSpike.myPosition, ratio * radius, 0.0f, Vec4f(1, 1, 0, 1));

			auto scene = myBoss.GetGameObject()->GetScene();

			if(!currentSpike.myTelegraphObject)
			{
				Engine::GameObjectPrefab& spikeTelegraphPrefab = myVars.mySpikeTelegraphPrefab->Get();
				GameObject& spikeTelegraph = spikeTelegraphPrefab.Instantiate(*scene);
				spikeTelegraph.GetTransform().SetPosition(currentSpike.myPosition);
				currentSpike.myTelegraphObject = &spikeTelegraph;
			}
			else
			{
				const auto& children = currentSpike.myTelegraphObject->GetTransform().GetChildren();

				if(children.size())
				{
					children[0]->SetScale(ratio);
				}
			}

			if (currentSpike.myTelegraphProgress <= 0.0f)
			{
				Engine::GameObjectPrefab& spikePrefab = myVars.mySpikeMeshPrefab->Get();
				GameObject& spike = spikePrefab.Instantiate(*scene);
				spike.GetTransform().SetPosition(currentSpike.myPosition);

				if(currentSpike.myTelegraphObject)
				{
					currentSpike.myTelegraphObject->Destroy();
					currentSpike.myTelegraphObject = nullptr;
				}

				if (auto extension = spike.GetComponent<BossSpikeExtension>())
				{
					extension->SetDamage(myVars.myMinDamage * myBoss.GetDmgMultiplier(), myVars.myMaxDamage * myBoss.GetDmgMultiplier());
					extension->SetForce(myVars.myForce);
				}
				mySpikes.erase(mySpikes.begin() + i);
			}
		}
	}
}
