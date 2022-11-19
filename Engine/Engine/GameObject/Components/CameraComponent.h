#pragma once

#include "Component.h"
#include "../../Renderer/RTransform.h"
#include "Engine/Renderer/Camera/Camera.h"

namespace Engine
{
	// CAN ONLY BE USED AS A MAIN CAMERA, NOT FOR ANYTHING
	// ELSE BECAUSE IT REGISTERS TO WINDOW RESIZED EVENT
	class CameraComponentNew : public Component
	{
	public:
		COMPONENT(CameraComponentNew, "CameraComponentNew");

		CameraComponentNew() = default;
		CameraComponentNew(GameObject* aGameObject);
		CameraComponentNew(const CameraComponentNew& aOther) = default;
		~CameraComponentNew() override;

		void InitCamera();

		void Execute(eEngineOrder aOrder) override;

		Camera& GetRendererCamera() { return myCamera; }

		void SetFov(float aDegrees);

		void Reflect(Reflector& aReflector) override;

	private:
		Camera myCamera;

		// Temporary for debugging
		bool myIsFreeCam = false;
		bool myIsFollowingEntity = false;

		// FOR DEBUGGING
		RTransform myFrozenFrustumTransform;
	};
}