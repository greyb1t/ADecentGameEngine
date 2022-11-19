#include "pch.h"
#include "MagicCircleHandler.h"
#include "Player/Player.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/Scene/Scene.h"
#include "Engine/ResourceManagement/Resources/GameObjectPrefabResource.h"
#include "Engine/GameObject/Prefab/GameObjectPrefab.h"
#include "Engine/GameObject/Components/ModelComponent.h"
#include "Engine/ResourceManagement/Resources/VFXResource.h"
#include "Engine/GameObject/Components/VFXComponent.h"
#include <algorithm>

#include "Engine/ResourceManagement/Resources/AnimationCurveResource.h"
#include "Game/Camera/CameraController.h"
#include "Engine/GameObject/Components/AnimatorComponent.h"

void MagicCircleHandler::Init(Player* aPlayer)
{
	myPlayer = aPlayer;
	
	auto scene = myPlayer->GetGameObject()->GetScene();

	myMagicHand = scene->AddGameObject<GameObject>();
	myMagicHand->SetName("Player Magic Circle");

	//myHandBoneIndex = myPlayer->GetAnimatorComponent().GetController().GetBoneIndexFromName("RightForeArm");

	auto muzzleFlash = myPlayer->GetVFXHolder().primary.fire;
	if (muzzleFlash && muzzleFlash->IsValid())
		myPrimaryMuzzleFlash = myMagicHand->AddComponent<Engine::VFXComponent>(muzzleFlash->Get());

	auto mainRune = myPlayer->GetVFXHolder().magicCircle.mainRune;
	if (mainRune && mainRune->IsValid())
	{
		myMainRune = &mainRune->Get().Instantiate(*scene);
		myMainModel = myMainRune->GetComponent<Engine::ModelComponent>();
		myMainRune->GetTransform().SetParent(&myMagicHand->GetTransform());
	}
	
	auto smallRune = myPlayer->GetVFXHolder().magicCircle.smallRune;
	if (smallRune && smallRune->IsValid())
	{
		mySmallRune = &smallRune->Get().Instantiate(*scene);
		mySmallModel = mySmallRune->GetComponent<Engine::ModelComponent>();
		mySmallRune->GetTransform().SetParent(&myMagicHand->GetTransform());
	}
}

void MagicCircleHandler::Update()
{
	myVisibilityTarget = myPlayer->GetStatus().isAiming ? 1 : 0;

	if (!myMainRune || !mySmallRune)
		return;
	//UpdatePosition();
	const auto pos = GetMuzzlePosition();
	const auto rot = GetMuzzleRotation();
	myMainRune->GetTransform().SetPosition(pos);

	myMagicHand->GetTransform().SetPosition(pos);
	myMagicHand->GetTransform().SetRotation(rot);

	mySmallRune->GetTransform().SetPositionLocal(mySmallOffset);

	UpdateVisiblity();

	if (myPrimary.isActive)
		UpdatePrimary();

	Vec3f direction = GetMuzzleForward();
	Quatf q = Quatf::AxisAngle(direction, myRotation * Math::Deg2Rad);
	myMainRune->GetTransform().SetRotationLocal(q);
	myMainRune->GetTransform().SetPositionLocal(Vec3f(0,0,-myPrimary.recoil));
}

GameObject* MagicCircleHandler::GetMagicHand()
{
	return myMagicHand;
}

Vec3f MagicCircleHandler::GetMuzzlePosition()
{
	return myPlayer->GetModelObject().GetTransform().GetPosition() + myOffset * GetMuzzleRotation();
}

Quatf MagicCircleHandler::GetMuzzleRotation()
{
	Vec3f forward = myPlayer->GetCameraController().GetCamera().Forward();
	float dot = Vec3f(0.f, 1.f, 0.f).Dot(forward);

	if(dot < 0.f)
	{
		return myPlayer->GetCameraController().GetCamera().GetRotation();
	}

	return myPlayer->GetTransform().GetRotation();
}

Vec3f MagicCircleHandler::GetMuzzleForward()
{
	Vec3f forward = myPlayer->GetCameraController().GetCamera().Forward();
	float dot = Vec3f(0.f, 1.f, 0.f).Dot(forward);

	if (dot < 0.f)
	{
		return forward;
	}

	return myPlayer->GetTransform().Forward();
}

Engine::VFXComponent* MagicCircleHandler::GetMuzzleFlash()
{
	return myPrimaryMuzzleFlash;
}

void MagicCircleHandler::Primary()
{
	myRotation = myPrimary.targetRotation;

	myPrimary.isActive = true;
	myPrimary.timer = 0;
	myPrimary.time = 1.f / (myPlayer->GetStats().GetAttackSpeed() * myPlayer->GetSkillStats().primary.attackSpeedMultiplier);
	myPrimary.startRotation = myRotation;
	myPrimary.targetRotation += myPlayer->GetSettings().magicCircle.primaryRotation.rotationAmount;

	if (myPrimary.targetRotation > 360) {
		myPrimary.targetRotation -= 360;
		myPrimary.startRotation -= 360;
		myRotation -= 360;
	}
}

Vec3f& MagicCircleHandler::OffsetRef()
{
	return myOffset;
}

Vec3f& MagicCircleHandler::OffsetSmallRef()
{
	return mySmallOffset;
}

void MagicCircleHandler::UpdatePosition()
{
	Mat4f boneTrans = myPlayer->GetAnimatorComponent().GetController().GetBoneTransformWorld(myHandBoneIndex);
	Vec3f translation;
	Quatf rotation;
	Vec3f scale;
	boneTrans.Decompose(translation, rotation, scale);

	const Vec3f targetPos = translation;// +myPlayer->GetTransform().GetMovement();
	//const Vec3f lerpPos = C::Lerp(myForearm->GetTransform().GetPosition(), targetPos, Time::DeltaTime * 20);
	//myMagicHand->GetTransform().SetRotation(rotation);
	const auto matrix = rotation.ToMatrix();
	const Vec3f offset = matrix.GetForward() * myPlayer->GetSettings().rightHand.offset.z
		+ matrix.GetUp() * myPlayer->GetSettings().rightHand.offset.y
		+ matrix.GetRight() * myPlayer->GetSettings().rightHand.offset.x;

	myMagicHand->GetTransform().SetPosition(targetPos + offset);
}

void MagicCircleHandler::UpdateVisiblity()
{
	const float delta = (1 / (myVisibilityTarget > 0.5f ? myPlayer->GetSettings().magicCircle.timeToVisible : -myPlayer->GetSettings().magicCircle.timeToInvisible)) * Time::DeltaTime;

	myVisiblity += delta;
	myVisiblity = std::clamp<float>(myVisiblity, 0, 1);

	if (!myPlayer->GetModelComponent().IsActive())
		myVisiblity = 0;

	if (myMainModel && mySmallModel) 
	{
		myMainModel->GetMeshMaterialInstanceByIndex(0).SetFloat4("myVar", Vec4f(0, 0, myVisiblity * 15, myVisiblity));
		mySmallModel->GetMeshMaterialInstanceByIndex(0).SetFloat4("myVar", Vec4f(0, 0, myVisiblity * 15, myVisiblity));
	}
}

void MagicCircleHandler::UpdatePrimary()
{
	myPrimary.timer += Time::DeltaTime;

	const float percentage = myPrimary.timer / myPrimary.time;
	if (percentage > 1.f) {
		EndPrimary();
		return;
	}

	auto rotationLerp = myPlayer->GetSettings().magicCircle.primaryRotation.rotationLerp;
	if (rotationLerp && rotationLerp->IsValid())
	{
		myRotation = myPrimary.startRotation + (myPlayer->GetSettings().magicCircle.primaryRotation.rotationAmount * rotationLerp->Get().Evaluate(percentage));
	}
	else {
		myRotation = C::Lerp(myPrimary.startRotation, myPrimary.targetRotation, percentage);
	}

	auto recoilLerp = myPlayer->GetSettings().magicCircle.primaryRotation.rotationLerp;
	if (recoilLerp && recoilLerp->IsValid())
	{
		myPrimary.recoil = myPlayer->GetSettings().magicCircle.primaryRotation.recoilStrength * recoilLerp->Get().Evaluate(percentage);
	}
	else {
		myPrimary.recoil = C::Lerp(0.f, myPlayer->GetSettings().magicCircle.primaryRotation.recoilStrength, percentage);
	}
}

void MagicCircleHandler::EndPrimary()
{
	myPrimary.isActive = false;
	myRotation = myPrimary.targetRotation;
	myPrimary.recoil = 0;
}
