#pragma once

#include "Engine/GameObject/GameObjectRef.h"
#include "Engine/GameObject/Components/Component.h"
#include "Engine/GameObject/Transform.h"

namespace Engine
{
	class FollowBoneComponent : public Component
	{
	public:
		COMPONENT(FollowBoneComponent, "FollowBoneComponent");

		FollowBoneComponent() = default;
		FollowBoneComponent(GameObject* aGameObject);

		void Execute(eEngineOrder aOrder) override;

		void Reflect(Reflector& aReflector) override;
		void FollowBone();
	private:
		std::string myBoneToFollow;
		GameObjectRef myFollowUUID;

		Vec3f myPivot;
		Vec3f myRotationOffset;
		Vec3f myPositionOffset;


		bool myFollowPosition = true;
		bool myFollowRotation = true;
		bool myFollowScale = true;
	};
}