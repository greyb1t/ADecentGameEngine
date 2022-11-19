#include "pch.h"
#include "PlayerSpawnHandler.h"
#include "Player.h"
#include "Camera/CameraController.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/GameObject/Prefab/GameObjectPrefab.h"
#include "Engine/GameObject/Components/CameraShakeComponent.h"

constexpr float fallingDistance = 16000.f;
constexpr float fallingTime = 2.4f;
constexpr float finishZoomTime = 1.f;

PlayerSpawnHandler::PlayerSpawnHandler(GameObject* aGameObject) : 
	Component(aGameObject)
{ }

void PlayerSpawnHandler::Init(Player* aPlayer, CameraController* aPlayerCamera)
{
	myPlayer = aPlayer;
	myPlayerCamera = aPlayerCamera;

	myCamera = myGameObject->GetScene()->FindGameObject("MainCamera");

	const GameObjectPrefabRef prefab = GResourceManager->CreateRef<Engine::GameObjectPrefabResource>("Assets/Prefabs/Comet.prefabg");
	prefab->Load();
	myCometObject = &Engine::GameObjectPrefab::InstantiatePrefabInstance(prefab, *myGameObject->GetScene());
	myCometObject->GetTransform().SetPosition(myPlayer->GetTransform().GetPosition() + CU::Vector3f{0.f, fallingDistance, 0.f});
	myCometObject->GetTransform().SetRotation(CU::Quaternion({ 0.f, 0.f, CU::HALF_PI }));
}

void PlayerSpawnHandler::Execute(Engine::eEngineOrder aOrder)
{
	if (!myPlayer)
	{
		LOG_ERROR(LogType::Bjorn) << "Player spawn handler isn't properly initialized";
		return;
	}
	if (myFirstFrame)
	{
		myTargetCameraPosition = myPlayerCamera->GetTransform().GetPosition();
		myPlayerCamera->SetIscontrollingCamera(false);

		CU::Vector3f flatBackwardsDirection = myPlayerCamera->GetCamera().Forward() * -1.f;
		flatBackwardsDirection.y = 0.f;
		flatBackwardsDirection.Normalize();
		myCamera->GetTransform().SetPosition(myPlayerCamera->GetTransform().GetPosition() + flatBackwardsDirection * 1100.f);

		myStartCameraPosition = myCamera->GetTransform().GetPosition();
		myFirstFrame = false;
		return;
	}

	if (!myHasArrived)
	{
		CU::Vector3f direction = myPlayer->GetTransform().GetPosition() - myCometObject->GetTransform().GetPosition();
		const float length = direction.Length();

		constexpr float cometSpeed = fallingDistance / fallingTime;
		direction = (direction / length) * CU::Min((cometSpeed * Time::DeltaTime), length);

		myCometObject->GetTransform().SetPosition(myCometObject->GetTransform().GetPosition() + direction);
		CU::Vector3f up{ 0.f, 1.f, 0.f };
		myCometObject->GetTransform().Rotate(CU::Quaternion::AxisAngle(up, 10.f * Time::DeltaTime));
		myCamera->GetTransform().LookAt(myCometObject->GetTransform().GetPosition());
		if (length < cometSpeed * Time::DeltaTime)
		{
			myPlayer->FinishRespawnSequence();
			myHasArrived = true;
			myCometObject->Destroy();
			auto camShake = myCamera->GetComponent<Engine::CameraShakeComponent>();
			camShake->AddPerlinShake("PlayerSpawnShake");
			ReturnControl();
			myGameObject->Destroy();
		}
	}
	/*else
	{
		myZoomCounter += Time::DeltaTime;

		if (myZoomCounter > finishZoomTime)
		{
			ReturnControl();
			myGameObject->Destroy();
		}
		else
		{
			myCamera->GetTransform().SetPosition(CU::Lerp(myStartCameraPosition, myTargetCameraPosition, myZoomCounter / finishZoomTime));
		}
	}*/


}

void PlayerSpawnHandler::ReturnControl()
{
	myPlayerCamera->SetIscontrollingCamera(true);
}
