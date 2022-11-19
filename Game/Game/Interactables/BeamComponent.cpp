#include "pch.h"
#include "BeamComponent.h"

#include "Engine\GameObject\GameObject.h"
#include "Engine\GameObject\Components\ModelComponent.h"

void BeamComponent::Awake()
{

}

void BeamComponent::Start()
{
	myPlayer = &Main::GetPlayer()->GetTransform();
	myStartPos = GetTransform().GetPosition();

	myModel = myGameObject->GetComponent<Engine::ModelComponent>();
	if (!myModel)
		myGameObject->SetActive(false);
	GetTransform().SetRotation(CU::Quaternion::Identity());
}

void BeamComponent::Reflect(Engine::Reflector& aReflector)
{
	aReflector.Reflect(myMaxDistance, "Closest Seen Distance");
	aReflector.Reflect(myMinDistance, "Distance when to start dissapearing");
	aReflector.Reflect(myTargetHeight, "Target Height");
}

void BeamComponent::Execute(Engine::eEngineOrder aOrder)
{
	Vec3f playerPosition = myPlayer->GetPosition();
	playerPosition.y = myStartPos.y;

	float distance = (playerPosition - myStartPos).Length();

	if (distance > myMinDistance)
		return;

	float percent = (distance - myMaxDistance) / (myMinDistance - myMaxDistance);
	percent = std::max(percent, 0.0f);

	float negPercent = 1.0f - percent;

	Vec3f myPosition = GetTransform().GetPosition();
	myPosition = myStartPos + (Vec3f(0.0f, myTargetHeight, 0.0f) * negPercent);
	GetTransform().SetPosition(myPosition);


	Vec3f scale = GetTransform().GetScale();
	scale.x = percent;
	scale.y = percent;
	scale.z = percent;

	GetTransform().SetScale(scale);


	if (scale.x <= 0.0f && myModel->IsActive())
	{
		myModel->SetActive(false);
	}
	else if (scale.x > 0.0f && !myModel->IsActive())
	{
		myModel->SetActive(true);
	}
}