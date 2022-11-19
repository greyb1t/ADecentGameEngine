#pragma once
#include "Component.h"
#include "Physics/RigidBody.h"

namespace Engine
{
	class RigidBodyComponent :
		public Component, public RigidBody
	{
	public:
		RigidBodyComponent();
		RigidBodyComponent(GameObject* aGameObject, eRigidBodyType aType = eRigidBodyType::DEFAULT);
		virtual ~RigidBodyComponent();

		Component* Clone() const override;

		void Start() override;
		void Execute(eEngineOrder aOrder) override;

		// WARNING: Offset is not rotated with the transform currently***
		void SetOffset(const CU::Vector3f& aOffset);
		const CU::Vector3f& GetOffset() const;
		void SetLocalRotation(const CU::Quaternion& aRotation);
		const CU::Quaternion& GetLocalRotation() const;

		void ObserveCollision(const std::function<void(GameObject*)>& aFunction);
		void ObserveTriggerStay(const std::function<void(GameObject*)>& aFunction);
		void ObserveTriggerEnter(const std::function<void(GameObject*)>& aFunction);
		void ObserveTriggerExit(const std::function<void(GameObject*)>& aFunction);
	private:
		void EarlyPhysics(float aDeltaTime);
		void LatePhysics(float aDeltaTime);

		void DebugDraw();

		std::function<void(GameObject*)> myTriggerEnterObserver;
		std::function<void(GameObject*)> myTriggerStayObserver;
		std::function<void(GameObject*)> myTriggerExitObserver;
		std::function<void(GameObject*)> myCollisionObserver;

		CU::Vector3f myOffset{ 0,0,0 };
		CU::Quaternion myLocalRotation;

	};
}

