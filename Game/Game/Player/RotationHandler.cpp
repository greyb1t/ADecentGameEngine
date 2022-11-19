#include "pch.h"
#include "RotationHandler.h"
#include "Engine/GameObject/Transform.h"
#include "Engine/GameObject/GameObject.h"
#include "Player.h"
#include "Camera/CameraController.h"

void RotationHandler::Init(Player* aPlayer)
{
	myPlayer = aPlayer;
	myRotation = aPlayer->GetModelObject().GetTransform().GetRotation();
}

void RotationHandler::Update()
{
	CU::Vector3f inputForward = myPlayer->GetInputDir();
	inputForward.z += 0.1f;

	Transform& cam = myPlayer->GetCameraController().GetCamera();
	Vec3f camForward = cam.Forward();
	camForward.y = 0.f;
	camForward.Normalize();
	Vec3f camRight = cam.Right();
	camRight.y = 0.f;
	camRight.Normalize();

	CU::Matrix4f rotationMatrix;
	rotationMatrix(1, 1) = camRight.x;
	rotationMatrix(1, 2) = camRight.y;
	rotationMatrix(1, 3) = camRight.z;
	rotationMatrix(3, 1) = camForward.x;
	rotationMatrix(3, 2) = camForward.y;
	rotationMatrix(3, 3) = camForward.z;

	CU::Quaternion goalRotation(rotationMatrix);
	myPlayer->GetTransform().SetRotation(goalRotation);

	bool shooting = myPlayer->GetStatus().isAiming;

	if (shooting)
	{
		rotationMatrix(1, 1) *= -1.f;
		rotationMatrix(1, 2) *= -1.f;
		rotationMatrix(1, 3) *= -1.f;
		rotationMatrix(3, 1) *= -1.f;
		rotationMatrix(3, 2) *= -1.f;
		rotationMatrix(3, 3) *= -1.f;
	}
	else
	{
		inputForward = (inputForward.ToVec4(0) * rotationMatrix).ToVec3();
		inputForward.Normalize();
		CU::Vector3f inputRight = CU::Vector3f{ 0.f, 1.f, 0.f }.Cross(inputForward);
		inputRight.Normalize();

		rotationMatrix(1, 1) = -inputRight.x;
		rotationMatrix(1, 2) = -inputRight.y;
		rotationMatrix(1, 3) = -inputRight.z;
		rotationMatrix(3, 1) = -inputForward.x;
		rotationMatrix(3, 2) = -inputForward.y;
		rotationMatrix(3, 3) = -inputForward.z;

	}
	goalRotation = CU::Quaternion(rotationMatrix);

	float angleDif = CU::Quaternion::Angle(myRotation, goalRotation);
	Transform& modelTransform = myPlayer->GetModelObject().GetTransform();
	if (angleDif > myMaxAngleDif)
	{
		const float catchUpPerSecond = shooting ? myCombatCatchUpPerSecond : myCatchUpPerSecond;
		CU::Quaternion newRot = CU::Quaternion::Slerp(myRotation, goalRotation, CU::Min(catchUpPerSecond * Time::DeltaTime, 1.f));
		modelTransform.SetRotation(newRot);
		myRotation = newRot;
	}
	else
	{
		modelTransform.SetRotation(goalRotation);
		myRotation = goalRotation;
	}

}
