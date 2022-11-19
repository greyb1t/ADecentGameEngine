#pragma once

#include "Component.h"

namespace Engine
{
	class EditorCameraMovement : public Component
	{
	public:
		COMPONENT(EditorCameraMovement, "EditorCameraMovement");

		EditorCameraMovement() = default;
		EditorCameraMovement(GameObject* aGameObject);

		void Execute(eEngineOrder aOrder) override;

		void SetSpeed(const float aSpeed);
		float GetSpeed() const;

		void SetCurrentRotation(const Vec3f& aCurrentRotation);
		const Vec3f& GetCurrentRotation() const;

		bool IsFlying() const;

	private:
		float mySpeed = 700.f;

		Vec3f myCurrentRotation;

		bool myIsRightClickDown = false;
	};
}