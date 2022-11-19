#pragma once

#include "Component.h"

namespace Engine
{
	class BasicMovementComponentNew : public Component
	{
	public:
		COMPONENT(BasicMovementComponentNew, "BasicMovementComponentNew");

		BasicMovementComponentNew() = default;
		BasicMovementComponentNew(GameObject* aGameObject);

		void Execute(eEngineOrder aOrder) override;

		void IsMoveActive(bool aState);
		bool GetIsMoveActive() const;

		void SetSpeed(const float aSpeed);
		float GetSpeed() const;

	private:
		bool myIsMoveActive = false;
		float Speed = 700.f;
		float RotationSpeed = 45.f;

		Vec3f CurrentRotationEuler;

		bool myIsRightClickDown = false;
	};
}