#include "pch.h"
#include "HandCrawler_Loop.h"
#include "Engine/GameObject/GameObject.h"
#include "AI/PollingStation/PollingStationComponent.h"
#include "Engine/GameObject/Components/AnimatorComponent.h"
#include "AI/FinalBoss/Constants.h"
#include "AI/FinalBoss/Behaviours/HandCrawler.h"
#include "Engine/AnimationCurve/Curve.h"
#include "Engine/ResourceManagement/Resources/AnimationCurveResource.h"
#include "AI/FinalBoss/FinalBossHand.h"
#include "Engine/GameObject/Components/RigidBodyComponent.h"
#include "AI/FinalBoss/FinalBoss.h"
#include "Components/HealthComponent.h"
#include "Engine/GameObject/Components/AudioComponent.h"

FB::HandCrawler_Loop::HandCrawler_Loop(HandCrawler& aParent)
	: myCrawler(aParent)
{
}

void FB::HandCrawler_Loop::Update()
{
	const auto& desc = myCrawler.Desc();

	StickToGround();

	myTimer.Tick(Time::DeltaTime);

	auto polling = myCrawler.GetBossHand().GetGameObject()->GetSingletonComponent<PollingStationComponent>();
	const auto playerPos = polling->GetPlayerPos();

	auto& transform = myCrawler.GetBossHand().GetGameObject()->GetTransform();
	const auto handPos = transform.GetPosition();

	const float dSq = desc.myStopSamplingPlayerPosDistance * desc.myStopSamplingPlayerPosDistance;

	switch (myState)
	{
	case State::HuntPlayer:
	{
		// Target Rotation
		// A float 1 to determine how much of the rotation has finished
		// Then use that float 1 to determine the walking speed

		if (myAttempts >= myMaxAttempts)
		{
			LOG_INFO(LogType::Game) << "Crawler reached max attempts, returning home";
			myState = State::ReturnToStart;
			break;
		}

		// if (myStartedAttackingPlayer)
		{
			/*
			const bool reachedSampledPlayerPos = Vec3f::DistanceSq(handPos, myNextTargetPos) < Constants::CloseEnough;

			if (reachedSampledPlayerPos)
			{
				myJumpStartPos = handPos;
				myJumpTargetPos = playerPos;
				myState = State::JumpTowardsPlayer;
			}
			*/
		}
		// else
		{
			if (ReachedNextPos())
			{

				if (DistSq2D(handPos, playerPos) < dSq)
				{
					mySampledPlayerPos = playerPos;
					SetNextTargetPos(playerPos);
					// myStartedAttackingPlayer = true;

					myJumpStartPos = handPos;
					myJumpTargetPos = playerPos;
					myState = State::JumpTowardsPlayer;

					const float extraDistance = 500.f;
					auto l = (playerPos - handPos).GetNormalized();
					auto t = playerPos + l * extraDistance;
					t.y = handPos.y;
					SetNextTargetPos(t);

					LOG_INFO(LogType::Game) << "Reached player, going to last sampled pos";
				}
				else
				{
					SetNextTargetPos(CreateZigZagPointTowardsPosition(playerPos));
					LOG_INFO(LogType::Game) << "New zig zag target pos";

					myAttempts++;
				}
			}
		}

		MoveTowardsNextPos();
	} break;
	case State::JumpTowardsPlayer:
	{
		// Walk in the direction of the player for
		playerPos;

		CheckPlayerCollision();

		// const float extraDistance = 500.f;
		// SetNextTargetPos(playerPos + (playerPos - handPos).GetNormalized() * extraDistance);

		// SetNextTargetPos(CreateZigZagPointTowardsPosition(myParent.GetBossHand().GetDefaultPos()));

		MoveTowardsNextPos();

		if (ReachedNextPos())
		{
			myState = State::ReturnToStart;
		}
	} break;
	case State::ReturnToStart:
	{
		myIsFinished = true;
		break;

		if (ReachedNextPos())
		{
			const bool closeEnoughToDestination =
				DistSq2D(handPos, myCrawler.GetBossHand().GetDefaultPos()) < dSq;

			if (closeEnoughToDestination)
			{
				SetNextTargetPos(myCrawler.GetBossHand().GetDefaultPos());

				LOG_INFO(LogType::Game) << "Reached initial position";
			}
			else
			{
				SetNextTargetPos(CreateZigZagPointTowardsPosition(myCrawler.GetBossHand().GetDefaultPos()));
				LOG_INFO(LogType::Game) << "New zig zag target pos";
			}
		}

		if (DistSq2D(handPos, myCrawler.GetBossHand().GetDefaultPos()) < Constants::CloseEnough)
		{
			myIsFinished = true;
		}

		MoveTowardsNextPos();
	} break;
	default:
		break;
	}
}

bool FB::HandCrawler_Loop::IsFinished()
{
	return myIsFinished;
}

void FB::HandCrawler_Loop::OnEnter()
{
	myHasHitPlayer = false;

	// Must do this here because cannot call virtual functions from constructor
	if (!myJumpSpeedCurve)
	{
		if (myCrawler.Desc().myJumpSpeedCurve &&
			myCrawler.Desc().myJumpSpeedCurve->IsValid())
		{
			myJumpSpeedCurve = &myCrawler.Desc().myJumpSpeedCurve->Get();
		}
		else
		{
			LOG_ERROR(LogType::Game) << "Hand crawler is missing jump speed curve";
		}
	}

	const auto& desc = myCrawler.Desc();

	myCrawler.GetBossHand().GetAnimator().GetController().SetInt(Constants::CrawlEyeStateIntName, 1);

	myIsFinished = false;

	myAttempts = 0;
	myMaxAttempts = Random::RandomInt(desc.myMaxZigZagAttemptsMin, desc.myMaxZigZagAttemptsMax);

	// myStartedAttackingPlayer = false;

	auto polling = myCrawler.GetBossHand().GetGameObject()->GetSingletonComponent<PollingStationComponent>();
	const auto playerPos = polling->GetPlayerPos();

	SetNextTargetPos(CreateZigZagPointTowardsPosition(playerPos));

	if (desc.myJumpSpeedCurve)
	{
		myJumpElapsedTime = desc.myJumpSpeedCurve->Get().GetMinimumTime();
		myJumpSpeedCurveMax = desc.myJumpSpeedCurve->Get().GetMaximumTime();
	}
	else
	{
		LOG_ERROR(LogType::Game) << "Crawler missing jump curve";
	}

	// myParent.GetRightHand().GetAnimator()->GetController().SetInt(Constants::GatlingEyeStateIntName, 2);
	// 
	// auto polling = myParent.GetRightHand().GetGameObject()->GetSingletonComponent<PollingStationComponent>();
	// const auto playerPos = polling->GetPlayerPos();

	myTimer = TickTimer::FromSeconds(
		Random::RandomFloat(
			desc.myHuntDurationRange.x,
			desc.myHuntDurationRange.y));

	// myJumpTimer = TickTimer::FromSeconds(1.f);

	myState = State::HuntPlayer;

	CrawlingSound(true);
}

void FB::HandCrawler_Loop::OnExit()
{
	CrawlingSound(false);
}

void FB::HandCrawler_Loop::StickToGround()
{
	auto& transform = myCrawler.GetBossHand().GetGameObject()->GetTransform();

	const auto handPos = transform.GetPosition();

	RayCastHit hit;

	const float upOffset = 50.f;

	if (myCrawler.GetBossHand().GetGameObject()->GetScene()->RayCast(
		handPos + (Vec3f(0.f, 1.f, 0.f) * upOffset),
		Vec3f(0.f, -1.f, 0.f),
		Constants::RaycastDistance,
		/*eLayer::NAVMESH*/eLayer::GROUND | eLayer::DEFAULT,
		hit))
	{
		// Move to ground
		auto pos = transform.GetPosition();
		pos.y = hit.Position.y + myCrawler.Desc().myOffsetAboveGround;
		transform.SetPosition(pos);
	}
	else
	{
		LOG_ERROR(LogType::Game) << "Finalboss raycast navmesh failed";
	}
}

Vec3f FB::HandCrawler_Loop::CreateZigZagPointTowardsPosition(const Vec3f& aTargetPos)
{
	const auto& desc = myCrawler.Desc();

	// Pick another zig zag target pos in direction of player
	const float randomAngleOffset = Random::RandomFloat(
		desc.myNextZigZagAngleOffsetRange.x,
		desc.myNextZigZagAngleOffsetRange.y);

	auto& transform = myCrawler.GetBossHand().GetGameObject()->GetTransform();
	const auto handPos = transform.GetPosition();

	const Vec3f dirToPlayer = (aTargetPos - handPos).GetNormalized();
	// const float angle = std::atan2f(dirToPlayer.x, dirToPlayer.z) /*+ randomAngleOffset*/;

	const Vec3f dirToPlayerOffseted = dirToPlayer * Quatf(Vec3f(0.f, randomAngleOffset, 0.f));

	const float posDistance = Random::RandomFloat(
		desc.myNextZigZagPositionDistanceMinMax.x,
		desc.myNextZigZagPositionDistanceMinMax.y);

	const Vec3f nextPos = handPos + (dirToPlayerOffseted * posDistance);

	return nextPos;
}

float NormalizeRadians(float aRadians)
{
	if (aRadians > Math::PI)
	{
		aRadians = aRadians - Math::PI * 2.f;
	}
	else if (aRadians < -Math::PI)
	{
		aRadians = aRadians + Math::PI * 2.f;
	}

	return aRadians;
}

void FB::HandCrawler_Loop::MoveTowardsNextPos()
{
	auto& transform = myCrawler.GetBossHand().GetGameObject()->GetTransform();
	const auto handPos = transform.GetPosition();

	const auto dirToTarget = (myNextTargetPos - handPos).GetNormalized();

	/*
	// Update rotation
	myCurrentRotationY = Math::LerpAngleRadians(myCurrentRotationY, myTargetRotY, Time::DeltaTime * 5.f);

	// transform.SetRotation(Vec3f(0.f, myCurrentRotationY, 0.f));
	myCrawler.GetBossHand().SetTargetRotation(Vec3f(0.f, myCurrentRotationY, 0.f));

	const float rotPercent = Math::InverseLerp(
		NormalizeRadians(myStartRotY),
		NormalizeRadians(myTargetRotY),
		NormalizeRadians(myCurrentRotationY));
	*/

	// Move towards sample player pos
	auto newPos = handPos;
	// save Y so we avoid modifying it when moving towards player
	float y = newPos.y;
	newPos += dirToTarget * myCrawler.Desc().myMoveSpeed * Time::DeltaTime /** rotPercent*/;
	newPos.y = y;
	transform.SetPosition(newPos);

	GDebugDrawer->DrawSphere3D(DebugDrawFlags::Gameplay, myNextTargetPos, 300.f);
}

bool FB::HandCrawler_Loop::ReachedNextPos() const
{
	auto& transform = myCrawler.GetBossHand().GetGameObject()->GetTransform();
	const auto handPos = transform.GetPosition();

	return DistSq2D(handPos, myNextTargetPos) < Constants::CloseEnough;
}

float FB::HandCrawler_Loop::DistSq2D(const Vec3f& aFirst, const Vec3f& aSecond) const
{
	// Does not check the Y axis
	const float d1 = Vec2f::DistanceSq(
		Vec2f(aFirst.x, aFirst.z),
		Vec2f(aSecond.x, aSecond.z));

	return d1;
}

void FB::HandCrawler_Loop::SetNextTargetPos(const Vec3f& aPos)
{
	auto& transform = myCrawler.GetBossHand().GetGameObject()->GetTransform();
	const auto handPos = transform.GetPosition();

	// Ignore the Y value because it will do the bug where it walks slow
	// due to having a direction in the air but it can only walk on the ground
	myNextTargetPos = Vec3f(aPos.x, handPos.y, aPos.z);

	const auto dirToTarget = (myNextTargetPos - handPos).GetNormalized();

	// myStartRotY = transform.GetRotation().EulerAngles().y;
	// myTargetRotY = -std::atan2f(dirToTarget.x, dirToTarget.z);
	myCrawler.GetBossHand().SetTargetRotation(Vec3f(0.f, -std::atan2f(dirToTarget.x, dirToTarget.z), 0.f), 10.f);
}

void FB::HandCrawler_Loop::CheckPlayerCollision()
{
	std::vector<OverlapHit> hits;

	auto rb = myCrawler.GetBossHand().GetColliderGameObject()->GetComponent<Engine::RigidBodyComponent>();

	if (!myHasHitPlayer)
	{
		// NOTE(filip): Must do box cast/sphere cast because the trigger does not work
		const bool hitSomething = myCrawler.GetBossHand().GetGameObject()->GetScene()->BoxCastAll(
			rb->GetTransform().GetPosition(),
			rb->GetTransform().GetRotation(),
			rb->GetShapes()[0].GetBoxHalfSize(),
			rb->GetGameObject()->GetDetectionLayerMask(),
			hits);

		if (hitSomething)
		{
			for (const auto& hit : hits)
			{
				if (hit.GameObject->GetTag() == eTag::PLAYER)
				{
					if (auto health = hit.GameObject->GetComponent<HealthComponent>())
					{
						LOG_ERROR(LogType::Filip) << "COLLIDEED";

						health->ApplyDamage(myCrawler.GetBossHand().GetHandCrawlerDesc().myDamage.GetDamage());

						const auto upStrength = Vec3f(0, 1, 0) * myCrawler.GetBossHand().GetHandCrawlerDesc().myKnockbackForce;
						const auto punchDirectionStrength = myCrawler.GetBossHand().GetTransform().Forward() * myCrawler.GetBossHand().GetHandCrawlerDesc().myKnockbackForce;
						health->ApplyKnockback(punchDirectionStrength + upStrength);

						myHasHitPlayer = true;
					}
				}
			}
		}
	}
}

void FB::HandCrawler_Loop::CrawlingSound(const bool aEnable)
{
	if (aEnable)
	{
		myCrawler.GetBossHand().GetAudio().PlayEvent("CrawlingHand");
	}
	else
	{
		myCrawler.GetBossHand().GetAudio().StopEvent("CrawlingHand");
	}
}
