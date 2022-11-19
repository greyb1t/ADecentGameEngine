#include "pch.h"
#include "NavMeshCreator.h"

#include "Engine/GameObject/Components/RigidStaticComponent.h"
#include "../Navmesh/NavMeshImporter.h"
#include "Physics/Physics.h"
#include "Physics/Shape.h"

void NavMeshCreator::SetupNavMesh(GameObject* aGameObject, NavMesh::NavMesh* aNavMesh)
{
	if (aNavMesh->GetVertices().empty())
	{
		LOG_ERROR(LogType::AIPlacement) << "No navmesh found for scene. Early returning. No enemies can spawn without a navmesh";
		return;
	}

	auto& navMeshVertices = aNavMesh->GetVertices();
	auto& navMeshTriangles = aNavMesh->GetTriangles();

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
	
	aGameObject->SetLayers(eLayer::NAVMESH);
	// TODO: Use custom create function in shape instead
	Shape shape = Shape::TriangleMesh(vertices, indices);

	shape.SetupFiltering(eLayer::NAVMESH, eLayer::ALL, eLayer::NONE);

	auto* rs = aGameObject->AddComponent<Engine::RigidStaticComponent>();
	rs->Attach(shape);
}