#pragma once

#include "Component.h"
#include "Engine\CameraShake/PerlinShake.h"

class GameObject;

// Save the shakers and undo them at the end, because if we stored the cam pos,
// it would not work for a moving camera

namespace Engine
{
	class CameraComponentNew;
	class CameraShakeBase;

	struct PerlinShakeDesc;
	struct KickShakeDesc;

	class CameraShakeComponent : public Component
	{
	public:
		COMPONENT(CameraShakeComponent, "Camera Shake");

		CameraShakeComponent() = default;
		CameraShakeComponent(GameObject* aGameObject);
		CameraShakeComponent(const CameraShakeComponent& aOther);

		void Start() override;

		void Execute(eEngineOrder aOrder) override;

		void AddPerlinShake(const PerlinShakeDesc& aDesc);
		void AddPerlinShake(const std::string& aShakeName);
		void AddPerlinShakeByDistance(const std::string& aShakeName, const float aMaxDistance, const float aCurrentDistance);
		void AddKickShake(const KickShakeDesc& aDesc, const Vec3f& aDirection);
		void AddKickShake(const std::string& aShakeName, const Vec3f& aDirection);


		void Reflect(Reflector& aReflector) override;

	private:
		CameraComponentNew* myCameraComponent = nullptr;
		std::vector<Owned<CameraShakeBase>> myActiveShakes;
	};
}