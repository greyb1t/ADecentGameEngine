#include "pch.h"
#include "ChestSpawnerComponent.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/Navmesh/NavmeshComponent.h"
#include "Engine/Navmesh/NavMesh.h"
#include "Engine/GameObject/Prefab/GameObjectPrefab.h"

void ChestSpawnerComponent::Start()
{
	Main::SetCanOpenChestShield(true);
}

void ChestSpawnerComponent::Execute(Engine::eEngineOrder anOrder)
{
	if (myHasStarted)
	{
		return;
	}

	GameObject* navmeshObj = myGameObject->GetScene()->FindGameObject("Navmesh");
	Engine::NavmeshComponent* navmeshComp = nullptr;
	if (navmeshObj)
	{
		navmeshComp = navmeshObj->GetComponent<Engine::NavmeshComponent>();
	}

	assert(navmeshComp && "NAME THE NAVMESH OBJECT \"Navmesh\" or things will crash");

	if (navmeshComp)
	{
		NavMesh::NavMesh* navmesh = navmeshComp->GetNavmesh();
		if (!navmesh)
		{
			LOG_WARNING(LogType::Items) << "Attempted to place a chest, but there was no navmesh :(";
			return;
		}
		std::vector<std::pair<CU::Vector3f, CU::Vector3f>> usedPositions;
		Engine::Scene* currentScene = myGameObject->GetScene();
		for (int i = 0; i < myNumberOfChests + 1; ++i) //+ 1 for the shrine
		{
			std::pair<CU::Vector3f, CU::Vector3f> posAndUp;
			auto isTooClose = [&](const std::pair<Vec3f, Vec3f>& aPosition)->bool {return (aPosition.first - posAndUp.first).LengthSqr() < myChestMinDistance * myChestMinDistance; };
			bool foundValidPos = false;
			posAndUp.first = navmesh->GetRandomPosition();
			RayCastHit hit;
			if (currentScene->RayCast(posAndUp.first + CU::Vector3f{ 0.f, 10000.f, 0.f }, { 0.f, -1.f, 0.f }, 11000.f, eLayer::ALL ^ eLayer::GROUND , hit, false))
			{
				if (CU::Abs(hit.Normal.Dot({ 0.f, 1.f, 0.f })) > 0.95f)
				{
					posAndUp.first = hit.Position;
					foundValidPos = true;
					foundValidPos = std::find_if(usedPositions.begin(), usedPositions.end(), isTooClose) == usedPositions.end();
					posAndUp.second = hit.Normal;
				}
			}

			if (foundValidPos)
			{
				usedPositions.push_back(posAndUp);
			}
			else
			{
				--i;
			}

		}

		auto& shrinePos = usedPositions.back();

		{
			GameObject& shrine = myShrinePrefab.myResource->Get().Instantiate(*myGameObject->GetScene());
			shrine.GetTransform().SetPosition(shrinePos.first);
			shrine.GetTransform().SetRotation(CreateRotationFromUp(shrinePos.second));
		}
		usedPositions.pop_back();

		for (auto& position : usedPositions)
		{
			float random = Random::RandomFloat(0.0f, 1.0f);

			if (random <= myRareChestPercent)
			{
				GameObject& object = myChestRarePrefab.myResource->Get().Instantiate(*myGameObject->GetScene());
				object.GetTransform().SetPosition(position.first);
				object.GetTransform().SetRotation(CreateRotationFromUp(position.second));
			}
			else
			{
				GameObject& object = myChestPrefab.myResource->Get().Instantiate(*myGameObject->GetScene());
				object.GetTransform().SetPosition(position.first);
				object.GetTransform().SetRotation(CreateRotationFromUp(position.second));

			}
		}

	}
	myHasStarted = true;
}

void ChestSpawnerComponent::Reflect(Engine::Reflector& aReflector)
{
	aReflector.Reflect(myNumberOfChests, "Number of chests"); 
	aReflector.Reflect(myChestMinDistance, "Minimum distance between chests");
	aReflector.Reflect(myRareChestPercent, "Rare Chest Spawn Percent (0-1)");
	aReflector.Reflect(myChestPrefab, "Chest Prefab");
	aReflector.Reflect(myChestRarePrefab, "Rare Chest Prefab");
	aReflector.Reflect(myShrinePrefab, "Shrine Prefab");
}

const CU::Quaternion ChestSpawnerComponent::CreateRotationFromUp(const CU::Vector3f& anUpDirection) const
{
	Mat4f rotMatrix;
	rotMatrix(2, 1) = anUpDirection.x;
	rotMatrix(2, 2) = anUpDirection.y;
	rotMatrix(2, 3) = anUpDirection.z;

	CU::Vector3f right(Random::RandomFloat(-10.f, 10.f), 0.f, Random::RandomFloat(-10.f, 10.f));
	right.Normalize();
	
	Vec3f forward = right.Cross(anUpDirection);
	forward.Normalize();
	
	right = anUpDirection.Cross(forward);
	right.Normalize();

	rotMatrix(1, 1) = right.x;
	rotMatrix(1, 2) = right.y;
	rotMatrix(1, 3) = right.z;

	rotMatrix(3, 1) = forward.x;
	rotMatrix(3, 2) = forward.y;
	rotMatrix(3, 3) = forward.z;

	return CU::Quaternion(rotMatrix);
}
