#include "pch.h"
#include "CharacterControllerComponent.h"
#include "../GameObject.h"
#include "Engine/Engine.h"
#include "Engine\DebugManager\DebugDrawer.h"
#include "Engine/Reflection/Reflector.h"
#include "Engine/Physics/PhysicsDebugDrawer.h"
#include "Engine/Shortcuts.h"

Engine::CharacterControllerComponent::CharacterControllerComponent(GameObject* aGameObject)
	: Component(aGameObject), CharacterController(aGameObject->GetScene())
{
}

Engine::CharacterControllerComponent::CharacterControllerComponent(GameObject* aGameObject, float aHeight, float aRadius)
	: Component(aGameObject), CharacterController(aGameObject->GetScene())
{
	Init(aHeight, aRadius);
}

Engine::CharacterControllerComponent::~CharacterControllerComponent()
{
	myGameObject->GetTransform().SetPhysical(false);
	CharacterController::~CharacterController();
	if (myMoveFilter)
	{
		delete myMoveFilter;
		myMoveFilter = nullptr;
	}
}

void Engine::CharacterControllerComponent::Init(float aHeight, float aRadius)
{
	// TODO: REFACTOR THIS INIT
	myHeight = aHeight;
	myRadius = aRadius;

	myGameObject->GetTransform().SetPhysical(true);

		CharacterController::Init(myGameObject->GetTransform().GetPosition(), aHeight, aRadius);
	ConnectUUID(myGameObject->GetUUID());

	SetMovementSettings(.1f);
	SetupFiltering(myGameObject->GetLayer(), myGameObject->GetPhysicalLayerMask(), myGameObject->GetDetectionLayerMask());
}

void Engine::CharacterControllerComponent::Execute(eEngineOrder aOrder)
{
	ZoneScopedN("CharacterController::Execute");

	switch (aOrder)
	{
	case EARLY_PHYSICS: EarlyPhysics(Time::DeltaTime); return;
	case LATE_PHYSICS:	LatePhysics(Time::DeltaTime); DebugDraw();
	}
}
void Engine::CharacterControllerComponent::Reflect(Reflector& aReflector)
{
	Component::Reflect(aReflector);
	auto result = aReflector.Reflect(myMoveToYPos, "Player Y Pos");
}

bool Engine::CharacterControllerComponent::IsGrounded()
{
	return myGroundedByteBuffer;
}

void Engine::CharacterControllerComponent::ClearGrounded()
{
	myGroundedByteBuffer = 0x00;
}

void Engine::CharacterControllerComponent::SetOffset(const CU::Vector3f& aOffset)
{
	myOffset = aOffset;
}

const CU::Vector3f& Engine::CharacterControllerComponent::GetOffset() const
{
	return myOffset;
}

void Engine::CharacterControllerComponent::ObserveCollision(const std::function<void(GameObject*)>& aFunction)
{
	if (!myIsCollisionListener)
	{
		myScene->GetCollisionRegistry().RegisterUUID(myUUID, true);
	}
	myCollisionObserver = aFunction;
	myIsCollisionListener = true;
}

void Engine::CharacterControllerComponent::ObserveTriggerStay(const std::function<void(GameObject*)>& aFunction)
{
	if (!myIsCollisionListener)
	{
		myScene->GetCollisionRegistry().RegisterUUID(myUUID, true);
	}
	myTriggerStayObserver = aFunction;
	myIsCollisionListener = true;
}

void Engine::CharacterControllerComponent::ObserveTriggerEnter(const std::function<void(GameObject*)>& aFunction)
{
	if (!myIsCollisionListener)
	{
		myScene->GetCollisionRegistry().RegisterUUID(myUUID, true);
	}
	myTriggerEnterObserver = aFunction;
	myIsCollisionListener = true;
}

void Engine::CharacterControllerComponent::ObserveTriggerExit(const std::function<void(GameObject*)>& aFunction)
{
	if (!myIsCollisionListener)
	{
		myScene->GetCollisionRegistry().RegisterUUID(myUUID, true);
	}
	myTriggerExitObserver = aFunction;
	myIsCollisionListener = true;
}

void Engine::CharacterControllerComponent::ObserveOnMoveCollision(const std::function<void(GameObject*)>& aFunction)
{
	if (!myIsCollisionListener)
	{
		myScene->GetCollisionRegistry().RegisterUUID(myUUID, true);
	}
	myOnMoveCollisionObserver = aFunction;
	myIsCollisionListener = true;
}

void Engine::CharacterControllerComponent::SetGravityActive(bool isActive)
{
	myGravityActive = isActive;
}

void Engine::CharacterControllerComponent::SetGravity(float aGravity)
{
	myGravityForce = aGravity;
}

void Engine::CharacterControllerComponent::SetMaxFallVelocity(float aMaxVelocity)
{
	myMaxFallVelocity = abs(aMaxVelocity);
}

void Engine::CharacterControllerComponent::SetVelocity(const Vec3f& aVelocity)
{
	myVelocity = aVelocity;
}

void Engine::CharacterControllerComponent::Jump(float aForce)
{
	myVelocity.y = aForce;
	ClearGrounded();
}

const Vec3f& Engine::CharacterControllerComponent::GetCharacterVelocity() const
{
	return myVelocity;
}

void Engine::CharacterControllerComponent::SetUpdatesPerSecond(const int aUpdatesPerSecond)
{
	myUpdatesPerSecond = aUpdatesPerSecond;
}

void Engine::CharacterControllerComponent::EarlyPhysics(float aDeltaTime)
{
	if (!myController)
		return;

	myUpdateProgress += Time::DeltaTime;

	const auto layer = myGameObject->GetLayer();
	const auto physicalLayerMask = myGameObject->GetPhysicalLayerMask();
	const auto detectionLayerMask = myGameObject->GetDetectionLayerMask();

	if (myFilterGroup != layer
		|| myPhysicalFilterTargets != physicalLayerMask
		|| myDetectionFilterTargets != detectionLayerMask)
	{
		SetupFiltering(layer, physicalLayerMask, detectionLayerMask);
	}

	physx::PxExtendedVec3 pxTarget = myController->getPosition();

	auto& transform = myGameObject->GetTransform();

	auto movement = transform.GetMovement();
	if (myGravityActive)
	{
		myVelocity.y += -myGravityForce * aDeltaTime;
		myVelocity.y = std::max(myVelocity.y, -myMaxFallVelocity);
	}

	movement += myVelocity * aDeltaTime;
	
	transform.SetRotation(transform.GetRotation() * transform.GetRotated());
	transform.ResetRotate();
	if (myUpdateProgress > 1.f / (float)myUpdatesPerSecond || myUpdatesPerSecond < 0)
	{
		float timeSinceLastMove = myUpdateProgress;
		myUpdateProgress -= 1.f / (float)myUpdatesPerSecond;
		if (movement != CU::Vector3f(0, 0, 0) || transform.GetRotated().myVector != CU::Quaternion().myVector)
		{
			//LOG_INFO(LogType::Physics) << "Move";
			Move(movement, timeSinceLastMove);
			//LOG_INFO(LogType::Physics) << "End move";

			transform.ResetMovement();

			if (CharacterController::IsGrounded())
			{
				myVelocity.y = 0;
			}

			return;
		}
	}

	if ((transform.GetPosition() - CU::Vector3f(pxTarget.x, pxTarget.y, pxTarget.z)).LengthSqr() < 2.f)
		return;

	physx::PxExtendedVec3 pos = { physx::PxReal(transform.GetPosition().x + myOffset.x), physx::PxReal(transform.GetPosition().y + myOffset.y), physx::PxReal(transform.GetPosition().z + myOffset.z) };
	myController->setPosition(pos);
}

void Engine::CharacterControllerComponent::LatePhysics(float aDeltaTime)
{
	if (!myController)
		return;

	auto& transform = myGameObject->GetTransform();

	physx::PxExtendedVec3 pxTarget = myController->getPosition();
	transform.SetPosition(CU::Vector3f(pxTarget.x, pxTarget.y, pxTarget.z) - myOffset);

	bool groundedThisFrame = CharacterController::IsGrounded();
	if (groundedThisFrame)
		myGroundedByteBuffer |= 0x01;
	myGroundedByteBuffer <<= 1;


	auto& collisions = myScene->GetCollisionRegistry().GetData(myUUID);

	if (!collisions.empty())
	{
		for (auto& c : collisions)
		{
			auto& data = c.a.uuid == myUUID ? c.b : c.a;

			auto* obj = myGameObject->GetScene()->FindGameObject(data.uuid);

			switch (c.state)
			{
			case eCollisionState::OnEnter:
			{
				if (myTriggerEnterObserver)
				{
					auto* obj = myGameObject->GetScene()->FindGameObject(data.uuid);
					if (obj)
						myTriggerEnterObserver(obj);
				}
			}
			break;
			case eCollisionState::OnExit:
			{
				if (myTriggerExitObserver)
				{
					auto* obj = myGameObject->GetScene()->FindGameObject(data.uuid);
					if (obj)
						myTriggerExitObserver(obj);
				}
			}
			break;
			case eCollisionState::OnStay:
			{
				if (myTriggerStayObserver)
				{
					auto* obj = myGameObject->GetScene()->FindGameObject(data.uuid);
					if (obj)
						myTriggerStayObserver(obj);
				}
			}
			break;
			case eCollisionState::OnCollision:
			{
				if (myCollisionObserver)
				{
					auto* obj = myGameObject->GetScene()->FindGameObject(data.uuid);
					if (obj)
						myCollisionObserver(obj);
				}
			}
			break;
			}
		}
	}
}

void Engine::CharacterControllerComponent::DebugDraw()
{
	if (!GetEngine().GetDebugDrawerSettings().IsDebugFlagActive(DebugDrawFlags::Physics))
	{
		return;
	}

	auto pos = myGameObject->GetTransform().GetPosition() + myOffset;

	GDebugDrawer->DrawLine3D(DebugDrawFlags::Physics, pos + CU::Vector3f{ 0, myHeight * .5f, 0 }, pos - CU::Vector3f{ 0, myHeight * .5f, 0 }, 0, P_CCT_COLOR);
	GDebugDrawer->DrawSphere3D(DebugDrawFlags::Physics, pos + CU::Vector3f{ 0, myHeight * .5f, 0 }, myRadius, 0, P_CCT_COLOR);
	GDebugDrawer->DrawSphere3D(DebugDrawFlags::Physics, pos - CU::Vector3f{ 0, myHeight * .5f, 0 }, myRadius, 0, P_CCT_COLOR);
}
