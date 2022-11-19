#include "pch.h"
#include "CameraShakeComponent.h"
#include "CameraComponent.h"
#include "..\GameObject.h"
#include "Engine/CameraShake\CameraShakeBase.h"
#include "Engine/CameraShake\Displacement.h"
#include "Engine/Engine.h"
#include "Engine/CameraShake\CameraShakeManager.h"
#include "Engine/CameraShake\PerlinShake.h"
#include "Engine\CameraShake\KickShake.h"
#include "Engine\Reflection\Reflector.h"

Engine::CameraShakeComponent::CameraShakeComponent(GameObject* aGameObject)
	: Component(aGameObject)
{
}

Engine::CameraShakeComponent::CameraShakeComponent(const CameraShakeComponent& aOther)
	: Component(aOther)
{
	// Dont copy anything, because not needed
}

void Engine::CameraShakeComponent::Start()
{
	myCameraComponent = myGameObject->GetComponent<CameraComponentNew>();
}

void Engine::CameraShakeComponent::Execute(eEngineOrder aOrder)
{
	assert(myCameraComponent);

	auto& camera = myCameraComponent->GetRendererCamera();

	Displacement shakeDisplacement;

	for (auto& shake : myActiveShakes)
	{
		shake->Update(Time::DeltaTime);

		shakeDisplacement = shakeDisplacement + shake->GetDisplacement();
	}

	camera.SetOffsetPosition(shakeDisplacement.GetPosition());
	camera.SetOffsetRotationEuler(shakeDisplacement.GetRotationEuler());

	for (int i = static_cast<int>(myActiveShakes.size()) - 1; i >= 0; --i)
	{
		if (myActiveShakes[i]->IsFinished())
		{
			myActiveShakes[i] = std::move(myActiveShakes.back());
			myActiveShakes.pop_back();
		}
	}
}

void Engine::CameraShakeComponent::AddPerlinShake(const PerlinShakeDesc& aDesc)
{
	myActiveShakes.push_back(MakeOwned<PerlinShake>(aDesc));
}

void Engine::CameraShakeComponent::AddPerlinShake(const std::string& aShakeName)
{
	const auto& shake = GetEngine().GetCameraShakeManager().GetPerlinShake(aShakeName);
	AddPerlinShake(shake);
}

void Engine::CameraShakeComponent::AddPerlinShakeByDistance(const std::string& aShakeName, const float aMaxDistance,
	const float aCurrentDistance)
{
	const float distance = (GetTransform().GetPosition() - myGameObject->GetTransform().GetPosition()).Length();
	const float shakeProcent = 1.f - Math::InverseLerp(0.f, aMaxDistance, aCurrentDistance);

	const auto mainCam = myGameObject->GetScene()->GetMainCameraGameObject();

	auto shake = GetEngine().GetCameraShakeManager().GetPerlinShake(aShakeName);
	shake.myAxesStrength = shake.myAxesStrength.ScaledBy(shakeProcent, shakeProcent);

	AddPerlinShake(shake);

}

void Engine::CameraShakeComponent::AddKickShake(const KickShakeDesc& aDesc, const Vec3f& aDirection)
{
	myActiveShakes.push_back(MakeOwned<KickShake>(aDesc, Displacement({ }, aDirection)));
}

void Engine::CameraShakeComponent::AddKickShake(const std::string& aShakeName, const Vec3f& aDirection)
{
	const auto& shake = GetEngine().GetCameraShakeManager().GetKickShake(aShakeName);
	AddKickShake(shake, aDirection);
}

void Engine::CameraShakeComponent::Reflect(Reflector& aReflector)
{
	aReflector.ReflectLambda(
		[this]()
		{
			auto& myCameraShakeManager = GetEngine().GetCameraShakeManager();

			ImGui::Text("Camera Shake");

			static std::string selectedShakeName;

			static ShakeType selectedShakeType = ShakeType::PerlinShake;

			static Vec3f direction = Vec3f(1.f, 0.f, 0.f);

			const char* shakeNames[] =
			{
				"Perlin Shake",
				"Kick Shake"
			};

			if (ImGui::BeginCombo("Type", shakeNames[static_cast<int>(selectedShakeType)]))
			{
				for (int i = 0; i < std::size(shakeNames); ++i)
				{
					if (ImGui::Selectable(shakeNames[i], i == static_cast<int>(selectedShakeType)))
					{
						selectedShakeType = static_cast<ShakeType>(i);
					}
				}

				ImGui::EndCombo();
			}

			if (ImGui::BeginCombo("Shakes", selectedShakeName.c_str()))
			{
				std::vector<std::string> shakes;

				if (selectedShakeType == ShakeType::PerlinShake)
				{
					shakes = myCameraShakeManager.GetPerlinShakeNames();
				}
				else if (selectedShakeType == ShakeType::KickShake)
				{
					shakes = myCameraShakeManager.GetKickShakeNames();
				}

				for (const auto& name : shakes)
				{
					if (ImGui::Selectable(name.c_str(), selectedShakeName == name))
					{
						selectedShakeName = name;
					}
				}

				ImGui::EndCombo();
			}

			if (selectedShakeType == ShakeType::PerlinShake)
			{
			}
			else if (selectedShakeType == ShakeType::KickShake)
			{
				ImGui::DragFloat3("Direction", &direction.x, 0.01f);
			}

			if (ImGui::Button("Play Shake"))
			{
				if (!selectedShakeName.empty())
				{
					auto mainCameraGo = myGameObject->GetScene()->GetMainCameraGameObject();
					auto shakeComponent = mainCameraGo->GetComponent<CameraShakeComponent>();

					switch (selectedShakeType)
					{
					case ShakeType::PerlinShake:
						shakeComponent->AddPerlinShake(selectedShakeName);
						break;
					case ShakeType::KickShake:
						shakeComponent->AddKickShake(selectedShakeName, direction);
						break;
					default:
						break;
					}
				}
			}

			/*
			if (ImGui::Button("test kick"))
			{
				auto mainCameraGo = myGameObject->GetScene()->GetMainCameraGameObject();
				auto shakeComponent = mainCameraGo->GetComponent<CameraShakeComponent>();

				KickShakeDesc desc;
				Displacement dir(Vec3f(0.f, -100.f, 0.f), Vec3f(0.f, 0.1f, 0.f));

				shakeComponent->AddKickShake(desc, dir);
			}
			*/
		});
}
