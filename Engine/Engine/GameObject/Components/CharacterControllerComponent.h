#pragma once
#include "Component.h"
#include "Physics\CharacterController.h"
#include "Engine/EngineOrder.h"

namespace Engine
{
	class CharacterControllerComponent :
		public Component, public CharacterController
	{
	public:
		COMPONENT_COPYABLE_MASK(CharacterControllerComponent, "CharacterControllerComponent", EARLY_PHYSICS | LATE_PHYSICS);

		CharacterControllerComponent() = default;
		CharacterControllerComponent(GameObject* aGameObject);
		CharacterControllerComponent(GameObject* aGameObject, float aHeight, float aRadius);
		~CharacterControllerComponent();

		void Init(float aHeight, float aRadius);

		void Execute(eEngineOrder aOrder) override;

		void Reflect(Reflector& aReflector) override;

		bool IsGrounded() override;
		void ClearGrounded();

		// WARNING: Offset is not rotated with the transform currently***
		void SetOffset(const CU::Vector3f& aOffset);
		const CU::Vector3f& GetOffset() const;

		void ObserveCollision(const std::function<void(GameObject*)>& aFunction);
		void ObserveTriggerStay(const std::function<void(GameObject*)>& aFunction);
		void ObserveTriggerEnter(const std::function<void(GameObject*)>& aFunction);
		void ObserveTriggerExit(const std::function<void(GameObject*)>& aFunction);
		void ObserveOnMoveCollision(const std::function<void(GameObject*)>& aFunction);

		void SetGravityActive(bool isActive);
		void SetGravity(float aGravity);

		void SetMaxFallVelocity(float aMaxVelocity);

		// Sets Y velocity
		void SetVelocity(const Vec3f& aVelocity);
		void Jump(float aForce);

		const Vec3f& GetCharacterVelocity() const;

		void SetUpdatesPerSecond(const int aUpdatesPerSecond);

	private:
		void EarlyPhysics(float aDeltaTime);
		void LatePhysics(float aDeltaTime);

		void DebugDraw();
	private:
		bool								myIsCollisionListener;
		std::function<void(GameObject*)>	myOnMoveCollisionObserver;
		std::function<void(GameObject*)>	myTriggerEnterObserver;
		std::function<void(GameObject*)>	myTriggerStayObserver;
		std::function<void(GameObject*)>	myTriggerExitObserver;
		std::function<void(GameObject*)>	myCollisionObserver;

		float myUpdateProgress = 0.f;
		int myUpdatesPerSecond = 60;

		CU::Vector3f myOffset{ 0, 0, 0 };

		bool myGravityActive = true;
		float myGravityForce = 981.f;
		float myMaxFallVelocity = 981.f * 5.f;

		float myMoveToYPos = 0;

		float myVelocityY = 0.f;
		Vec3f myVelocity;

		unsigned char myGroundedByteBuffer = 0;
	};
}