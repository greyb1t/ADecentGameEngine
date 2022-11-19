#include "pch.h"
#include "EngageState.h"

#include "Game/Player/Player.h"

LevelBossStates::EngageState::EngageState(LevelBoss& aBoss) : BaseState(aBoss)
{
}

void LevelBossStates::EngageState::Update()
{
	auto pollingStation = myBoss.GetGameObject()->GetSingletonComponent<PollingStationComponent>();
	if (!pollingStation)
		return;

	Vec3f playerPosition = pollingStation->GetPlayerPos();
	myState == State::Idle ? OnIdle(playerPosition) : OnAttack(playerPosition);
}

void LevelBossStates::EngageState::TurnTowards(const Vec3f& aTargetPosition, float aSpeed)
{
	Vec3f position = GetTransform().GetPosition();
	Vec3f direction = (aTargetPosition - position).GetNormalized();


	const float angle = std::atan2f(direction.x, direction.z) + CU::PI;

	Quatf startRot = GetTransform().GetRotation();
	Quatf endRot = Quatf(Vec3f(
		startRot.EulerAngles().x,
		-angle,
		startRot.EulerAngles().z
	));

	// Framerate independant, men fungerar inte när man roterar över 360 grader, då tar den omvägen
	//Vec3f euler = Math::SmoothDamp(startRot.EulerAngles(), endRot.EulerAngles(), mySmoothDampVelocity, 0.25f, 45.0f, Time::DeltaTime);
	//GetTransform().SetRotation(Quatf(euler));

	// Framerate dependant (yikes!), men fungerar i alla grader
	Quatf lerpRot = Quatf::Slerp(startRot, endRot, aSpeed * Time::DeltaTime);
	GetTransform().SetRotation(lerpRot);
}

PollingStationComponent* LevelBossStates::EngageState::GetPollingStation()
{
	return myBoss.GetGameObject()->GetSingletonComponent<PollingStationComponent>();
}

Vec3f LevelBossStates::EngageState::GetPredictedPosition()
{
	const auto& position = GetPollingStation()->GetPlayerPos();
	if (auto go = myBoss.GetPlayerObject())
	{
		if (auto player = go->GetComponent<Player>())
		{
			auto velocity = player->GetStatus().movementDirection;
			return position + velocity;
		}
	}
	return position;
}
