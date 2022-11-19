#include "pch.h"
#include "HandPunch.h"
#include "AI/FinalBoss/LeftPunchHand.h"
#include "AI/FinalBoss/States/HandPunch/HandPunch_Buildup.h"
#include "AI/FinalBoss/States/HandPunch/HandPunch_MoveTowardsPlayer.h"
#include "AI/FinalBoss/States/HandPunch/HandPunch_Returning.h"
#include "Engine/GameObject/GameObject.h"
#include "Components/HealthComponent.h"
#include "Engine/GameObject/Components/RigidBodyComponent.h"
#include "AI/FinalBoss/FinalBoss.h"
#include "Engine/GameObject/Components/CameraShakeComponent.h"
#include "Engine/GameObject/Components/AudioComponent.h"

FB::HandPunch::HandPunch(LeftPunchHand& aLeftHand)
	: myLeftHand(aLeftHand)
{
	myStates[HandPunchStateType::Buildup] = MakeOwned<HandPunch_Buildup>(*this);
	myStates[HandPunchStateType::MoveTowardsPlayer] = MakeOwned<HandPunch_MoveTowardsPlayer>(*this);
	myStates[HandPunchStateType::Returning] = MakeOwned<HandPunch_Returning>(*this);

	myStateSequencer.AddState(myStates[HandPunchStateType::Buildup].get());
	myStateSequencer.AddState(myStates[HandPunchStateType::MoveTowardsPlayer].get());
	myStateSequencer.AddState(myStates[HandPunchStateType::Returning].get());
}

void FB::HandPunch::Update()
{
	std::vector<OverlapHit> hits;

	auto rb = myLeftHand.GetColliderGameObject()->GetComponent<Engine::RigidBodyComponent>();

	if (!myHasHitPlayer)
	{
		// NOTE(filip): Must do box cast/sphere cast because the trigger does not work
		const bool hitSomething = myLeftHand.GetGameObject()->GetScene()->BoxCastAll(
			rb->GetTransform().GetPosition(),
			rb->GetTransform().GetRotation(),
			rb->GetShapes()[0].GetBoxHalfSize(),
			rb->GetGameObject()->GetDetectionLayerMask() | eLayer::DEFAULT /* DEFAULT to make pillars block */,
			hits);

		if (hitSomething)
		{
			for (int i = static_cast<int>(hits.size()) - 1; i >= 0; --i)
			{
				const auto& hit = hits[i];

				if (hit.GameObject->GetTag() == eTag::PLAYER)
				{
					if (auto health = hit.GameObject->GetComponent<HealthComponent>())
					{
						LOG_ERROR(LogType::Filip) << "Collided with player";

						health->ApplyDamage(myLeftHand.GetHandPunchDesc().myDamage.GetDamage());

						const auto upStrength = Vec3f(0, 1, 0) * myLeftHand.GetHandPunchDesc().myHitForce;
						const auto punchDirectionStrength = myLeftHand.GetTransform().Forward() * myLeftHand.GetHandPunchDesc().myHitForce;
						health->ApplyKnockback(punchDirectionStrength + upStrength);

						// cam shake
						{
							auto mainCam = myLeftHand.GetGameObject()->GetScene()->GetMainCameraGameObject();
							auto shakeComponent = mainCam->GetComponent<Engine::CameraShakeComponent>();
							if (shakeComponent)
							{
								// shakeComponent->AddPerlinShake("HandSlamHitGround");
								shakeComponent->AddKickShake("HandPunchHitPlayer", myLeftHand.GetTransform().Forward());
							}
						}

						myHasHitPlayer = true;
					}
				}
				else
				{
					LOG_ERROR(LogType::Filip) << "Collided with wall";

					if (myStateSequencer.GetActiveStateIndex() == HandPunchStateType::MoveTowardsPlayer)
					{
						auto moveTowardsPlayer = reinterpret_cast<HandPunch_MoveTowardsPlayer*>(myStateSequencer.GetActiveState());
						moveTowardsPlayer->OnCollidedWithObstacle();
					}

					// Avoid colling with other object such as the player
					break;
				}
			}
		}
	}

	myStateSequencer.Update();
}

bool FB::HandPunch::IsFinished() const
{
	return myStateSequencer.IsFinished();
}

FB::LeftPunchHand& FB::HandPunch::GetLeftHand()
{
	return myLeftHand;
}

const Vec3f& FB::HandPunch::GetInitialPosition() const
{
	return myInitialPosition;
}

void FB::HandPunch::HandPunchSound(const bool aEnable) const
{
	if (aEnable)
	{
		myLeftHand.GetAudio().PlayEvent("HandJab");
	}
	else
	{
		myLeftHand.GetAudio().StopEvent("HandJab");
	}
}

bool FB::HandPunch::HasHitPlayer() const
{
	return myHasHitPlayer;
}

void FB::HandPunch::OnEnter()
{
	myInitialPosition = myLeftHand.GetTransform().GetPosition();

	myStateSequencer.Init();
	myHasHitPlayer = false;
}

void FB::HandPunch::OnExit()
{
	myStateSequencer.OnExit();
}

void FB::HandPunch::Reflect(Engine::Reflector& aReflector)
{
	HandPunchStateType myStateType = HandPunchStateType::_from_integral(myStateSequencer.GetActiveStateIndex());
	aReflector.ReflectEnum(myStateType, "State");

	if (myStateSequencer.GetActiveState())
	{
		myStateSequencer.GetActiveState()->Reflect(aReflector);
	}
}

void FB::HandPunch::OnCollisionEnter(GameObject& aGameObject)
{

}

void FB::HandPunch::OnCollisionStay(GameObject& aGameObject)
{
	if (myHasHitPlayer)
	{
		return;
	}
}
