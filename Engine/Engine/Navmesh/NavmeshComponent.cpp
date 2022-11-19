#include "pch.h"
#include "NavmeshComponent.h"

#include "NavMeshCreator.h"
#include "NavMeshImporterBinary.h"
#include "NavMeshThreaded.h"
#include "Engine/GameObject/Components/RigidBodyComponent.h"
#include "Engine/GameObject/Components/RigidStaticComponent.h"
#include "Engine/Scene/FolderScene.h"
#include "Physics/Shape.h"

Engine::NavmeshComponent::NavmeshComponent(GameObject* aGameObject) :
Component(aGameObject)
{
}

Engine::NavmeshComponent::~NavmeshComponent()
{
	// NOTE(filip): MUST FREE, OTHERWISE ITS THREAD WILL NEVER STOP
	delete myNavMesh;
	myNavMesh = nullptr;

	delete myNavThread;
	myNavThread = nullptr;
}

void Engine::NavmeshComponent::Start()
{
	Component::Start();

	auto folderScene = dynamic_cast<FolderScene*>(myGameObject->GetScene());
	const auto navMeshPath = folderScene->GetFolderPath().AppendPath("NavMesh.bin");

	navMeshPath.ToWString();

	std::string navmeshStringBinary = navMeshPath.ToString();

	NavMesh::NavMeshImporterBinary navImporterBinary;

	if (!navImporterBinary.LoadNavMesh(navmeshStringBinary))
	{
		LOG_ERROR(LogType::Game) << "Failed to load navmesh " << navmeshStringBinary;
		return;
	}

	myNavMesh = navImporterBinary.CreateNavMesh(CU::Vector3f(0.f, 0.f, 0));

	NavMeshCreator::SetupNavMesh(myGameObject, myNavMesh);

	myNavThread = DBG_NEW NavMeshThreaded();
	myNavThread->Init();
	myNavThread->SetNavMesh(myNavMesh);

	/*myNavThread2 = DBG_NEW NavMeshThreaded();
	myNavThread2->Init();
	myNavThread2->SetNavMesh(myNavMesh);*/

	{
		auto& navMeshVertices = myNavMesh->GetVertices();
		auto& navMeshTriangles = myNavMesh->GetTriangles();

		std::vector<CU::Vector3f> vertices;
		std::vector<unsigned int> indices;

		for (auto& vertex : navMeshVertices)
		{
			vertices.emplace_back(vertex.myPosition);
		}

		for (auto& triangle : navMeshTriangles)
		{
			indices.emplace_back(triangle.myIndices[0]);
			indices.emplace_back(triangle.myIndices[1]);
			indices.emplace_back(triangle.myIndices[2]);
		}

		std::reverse(indices.begin(), indices.end());

		auto shape = Shape::TriangleMesh(vertices, indices);

		//auto* rs = myGameObject->AddComponent<RigidBodyComponent>(eRigidBodyType::KINEMATIC);

		myGameObject->SetLayers(eLayer::NAVMESH, eLayer::ALL, eLayer::NONE);
		auto* rs = myGameObject->AddComponent<RigidStaticComponent>();
		rs->Attach(shape);
	}
}

void Engine::NavmeshComponent::Execute(eEngineOrder aOrder)
{
	Component::Execute(aOrder);

	if (myNavMesh)
	{
		myNavMesh->DrawMesh();
	}

}

NavMesh::NavMesh* Engine::NavmeshComponent::GetNavmesh()
{
	return myNavMesh;
}

NavMeshThreaded& Engine::NavmeshComponent::GetNavThread()
{
	return *myNavThread;
}
//NavMeshThreaded& Engine::NavmeshComponent::GetNavThread2()
//{
//	return *myNavThread2;
//}
