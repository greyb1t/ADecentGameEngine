#include "pch.h"
#include "FollowAnimatedBoneComponent.h"

#include "Engine/GameObject/GameObject.h"
#include "Engine/GameObject/Components/AnimatorComponent.h"

Engine::FollowAnimatedBoneComponent::FollowAnimatedBoneComponent(GameObject* aGameObject)
	: Component(aGameObject)
{

}

void Engine::FollowAnimatedBoneComponent::Start()
{
	myAnimatorComponent = myGameObject->GetComponent<AnimatorComponent>();
}
