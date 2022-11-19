#include "pch.h"
#include "FollowBoneComponent.h"
#include "Engine/Engine.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/Reflection/Reflector.h"
#include "Engine/Renderer/GraphicsEngine.h"
#include "Engine/Renderer/WindowHandler.h"
#include "Engine/GameObject/Components/AnimatorComponent.h"
#include "Engine/GameObject/Components/CameraComponent.h"

Engine::FollowBoneComponent::FollowBoneComponent(GameObject* aGameObject)
	: Component(aGameObject)
{

}

void Engine::FollowBoneComponent::Execute(eEngineOrder aOrder)
{
	FollowBone();
}

void Engine::FollowBoneComponent::Reflect(Reflector& aReflector)
{
	Component::Reflect(aReflector);

	aReflector.Reflect(myFollowUUID, "Game Object to Follow");
	aReflector.Reflect(myBoneToFollow, "Bone To Follow");

	aReflector.Reflect(myPivot, "Pivot");
	aReflector.Reflect(myRotationOffset, "Rotation Offset");
	aReflector.Reflect(myPositionOffset, "Position Offset");

	aReflector.Reflect(myFollowPosition, "Follow Position");
	aReflector.Reflect(myFollowRotation, "Follow Rotation");
	aReflector.Reflect(myFollowScale, "Follow Scale");
}
void Engine::FollowBoneComponent::FollowBone()
{
	GameObject* gameObject = myFollowUUID.Get();
	if (gameObject == nullptr)
	{
		LOG_INFO(LogType::Components) << "This Game Object does not exist";
		return;
	}

	if (gameObject->GetComponent<AnimatorComponent>())
	{
		auto& controller = gameObject->GetComponent<AnimatorComponent>()->GetController();
		auto boneIndex = controller.GetBoneIndexFromName(myBoneToFollow);
		if (boneIndex < 0)
		{
			LOG_INFO(LogType::Components) << "Joint name not found in animation";
			return;
		}
		auto boneTransform = controller.GetBoneTransformWorld(boneIndex);

		Vec3f pos;
		Quatf rot;
		Vec3f scale;

		boneTransform.Decompose(pos, rot, scale);

		if (myFollowPosition)
		{
			myGameObject->GetTransform().SetPosition(pos + myPositionOffset);
		}
		if (myFollowRotation)
		{
			myGameObject->GetTransform().SetRotation(rot * Quatf(myRotationOffset * Math::DegToRad));
		}
		if (myFollowScale)
		{
			myGameObject->GetTransform().SetScale(scale);
		}
	}
	else
	{
		LOG_INFO(LogType::Components) << "This Game Object does not have an AnimatorComponent";
	}
}