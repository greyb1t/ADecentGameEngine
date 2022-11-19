#include "pch.h"
#include "FolliageCuller.h"
#include "../Reflection/Reflector.h"
#include "../GameObject.h"
#include "ModelComponent.h"
#include "EnvironmentLightComponent.h"

namespace Engine
{
	FolliageCuller::FolliageCuller(GameObject* aGameObject)
		: Component()
	{

	}

	void FolliageCuller::Start()
	{
	}

	void FolliageCuller::Render()
	{
		if (!myEnvLight)
		{
			myEnvLight = myGameObject->GetScene()->GetEnvironmentLight()->GetComponent<EnvironmentLightComponent>();
			return;
		}

		float dista = myEnvLight->GetFolliageCullingDistance();

		if(myDistanceOverridden)
		{
			dista = myDistance;
		}

		if (!myModel)
		{
			myModel = GetGameObject()->GetComponent<ModelComponent>();
		}

		auto& mainCamera = myGameObject->GetScene()->GetMainCamera();
		float dist = (GetTransform().GetPosition() - mainCamera.GetTransform().GetPosition()).LengthSqr();

		if (dist > dista * dista)
			myModel->SetActive(false);
		else
			myModel->SetActive(true);
	}

	void FolliageCuller::Reflect(Reflector& aReflector)
	{
		Component::Reflect(aReflector);

		aReflector.Reflect(myDistanceOverridden, "Override distance");

		if(myDistanceOverridden)
		{
			aReflector.Reflect(myDistance, "Distance");
		}
	}
}
