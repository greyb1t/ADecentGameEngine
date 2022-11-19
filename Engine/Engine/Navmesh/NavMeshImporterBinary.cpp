#include "pch.h"
#include "NavMeshImporterBinary.h"
#include "Engine\Engine.h"
#include "NavMesh.h"

bool NavMesh::NavMeshImporterBinary::LoadNavMesh(const std::string& aFilePath)
{
	if (!ReadIntoBuffer(aFilePath))
	{
		LOG_ERROR(LogType::Game) << "Failed to load navmesh " << aFilePath;
		return false;
	}

	myHeader = reinterpret_cast<BinaryNavmeshHeader*>(myBuffer.data());

	if (strcmp(myHeader->myMagic, "NAVG6") != 0)
	{
		LOG_ERROR(LogType::Game) << "Not a valid navmesh file, invalid header magic";
		return false;
	}

	ReadVertices();

	ReadTriangles();

	return true;
}

NavMesh::NavMesh* NavMesh::NavMeshImporterBinary::CreateNavMesh(const CU::Vector3f& aOffset) const
{
	NavMesh* navMesh = DBG_NEW NavMesh();

	for (const Vertex* vertex : myVertices)
	{
		Vertex v = *vertex;

		v.myPosition += aOffset;

		navMesh->myVertices.push_back(*vertex);
	}

	navMesh->myNumberOfTriangles = myTriangles.size();
	navMesh->myNumberOfVertices = myVertices.size();

	for (const NavMeshTriangle* triangle : myTriangles)
	{
		Triangle tri;

		// ALL THIS CODE IS JUST SAFE COPY CODE TO AVOID ISSUES IF SOMETHING IN THE STRUCTS CHANGE
		tri.myIndices[0] = triangle->myIndices[0];
		tri.myIndices[1] = triangle->myIndices[1];
		tri.myIndices[2] = triangle->myIndices[2];

		tri.myVetrices[0].myPosition.x = triangle->myVertices[0].x + aOffset.x;
		tri.myVetrices[0].myPosition.y = triangle->myVertices[0].y + aOffset.y;
		tri.myVetrices[0].myPosition.z = triangle->myVertices[0].z + aOffset.z;

		tri.myVetrices[1].myPosition.x = triangle->myVertices[1].x + aOffset.x;
		tri.myVetrices[1].myPosition.y = triangle->myVertices[1].y + aOffset.y;
		tri.myVetrices[1].myPosition.z = triangle->myVertices[1].z + aOffset.z;

		tri.myVetrices[2].myPosition.x = triangle->myVertices[2].x + aOffset.x;
		tri.myVetrices[2].myPosition.y = triangle->myVertices[2].y + aOffset.y;
		tri.myVetrices[2].myPosition.z = triangle->myVertices[2].z + aOffset.z;

		tri.myMiddlePos.x = triangle->myMiddlePos[0] + aOffset.x;
		tri.myMiddlePos.y = triangle->myMiddlePos[1] + aOffset.y;
		tri.myMiddlePos.z = triangle->myMiddlePos[2] + aOffset.z;

		navMesh->myTriangles.push_back(tri);
	}

	for (int i = 0; i < myTriangles.size(); ++i)
	{
		const NavMeshTriangle* triangle = myTriangles[i];

		for (int j = 0; j < triangle->myNeighboursCount; ++j)
		{
			Triangle* neighbour = &navMesh->myTriangles[triangle->myNeighbourIndices[j]];

			navMesh->myTriangles[i].myNeighbours.push_back(neighbour);
		}
	}

	return navMesh;
}

const std::vector<const NavMesh::Vertex*> NavMesh::NavMeshImporterBinary::GetVertices() const
{
	return myVertices;
}

const std::vector<const NavMesh::NavMeshImporterBinary::NavMeshTriangle*>
NavMesh::NavMeshImporterBinary::GetTriangles() const
{
	return myTriangles;
}

bool NavMesh::NavMeshImporterBinary::ReadIntoBuffer(const std::string& aFilePath)
{
	std::ifstream file(aFilePath, std::ifstream::binary);

	if (!file.is_open())
	{
		return false;
	}

	file.seekg(0, file.end);
	const int length = file.tellg();
	file.seekg(0, file.beg);

	myBuffer.resize(length);

	file.read(reinterpret_cast<char*>(myBuffer.data()), length);

	if (myBuffer.size() == 0)
	{
		return false;
	}

	return true;
}

void NavMesh::NavMeshImporterBinary::ReadVertices()
{
	myVertices.reserve(myHeader->myVerticesCount);

	const auto vertices =
		reinterpret_cast<Vertex*>(
			reinterpret_cast<uintptr_t>(myBuffer.data()) + myHeader->myVerticesFileOffset);

	for (int i = 0; i < myHeader->myVerticesCount; ++i)
	{
		myVertices.push_back(&vertices[i]);
	}
}

void NavMesh::NavMeshImporterBinary::ReadTriangles()
{
	myTriangles.reserve(myHeader->myTriangleCount);

	auto triangles =
		reinterpret_cast<InternalNavMeshTriangle*>(
			reinterpret_cast<uintptr_t>(myBuffer.data()) + myHeader->myTriangleFileOffset);

	for (int i = 0; i < myHeader->myTriangleCount; ++i)
	{
		myTriangles.push_back(reinterpret_cast<NavMeshTriangle*>(&triangles[i]));

		auto trisInt32 = reinterpret_cast<uint32_t*>(triangles);
		trisInt32 += triangles[i].myNeighboursCount;
		triangles = reinterpret_cast<InternalNavMeshTriangle*>(trisInt32);
	}
}
