#include "pch.h"
#include "NavMeshImporter.h"
#include <fstream>
#include <sstream>
#include "Engine\Engine.h"

NavMesh::NavMesh* NavMesh::NavMeshImporter::LoadNavMesh(std::string aFilePath)
{
	for (auto& mesh : myNavMeshes)
	{
		if (mesh.myPath == aFilePath)
		{
			return mesh.myNavMesh;
		}
	}


	std::vector<CU::Vector3i> vectors;
	std::vector<CU::Vector3i> faces;

	std::ifstream myfile(aFilePath);

	std::string line;
	while (getline(myfile, line))
	{
		if (line[0] == 'v')
		{
			std::string v, xPos, yPos, zPos;
			std::istringstream iss(line);

			iss >> v >> xPos >> yPos >> zPos;

			CU::Vector3i vector = { std::stoi(xPos), std::stoi(yPos), -std::stoi(zPos) };
			vectors.push_back(vector);
		}
		else if (line[0] == 'f')
		{
			std::string f, vertice1, vertice2, vertice3;
			std::istringstream iss(line);

			iss >> f >> vertice1 >> vertice2 >> vertice3;

			CU::Vector3i face = { std::stoi(vertice1), std::stoi(vertice2), std::stoi(vertice3) };
			faces.push_back(face);
		}
	}


	myNavMeshes.push_back(NavigationMesh());
	NavigationMesh& navMesh = myNavMeshes.back();
	navMesh.myNavMesh = DBG_NEW NavMesh();
	navMesh.myPath = aFilePath;

	for (CU::Vector3i& vector : vectors)
	{
		navMesh.myNavMesh->myVertices.push_back(Vertex());
		navMesh.myNavMesh->myVertices.back().myPosition = { static_cast<float>(vector.x), static_cast<float>(vector.y), static_cast<float>(vector.z) };
	}

	for (CU::Vector3i& face : faces)
	{

		navMesh.myNavMesh->myNumberOfTriangles = faces.size();
		navMesh.myNavMesh->myNumberOfVertices = vectors.size();

		navMesh.myNavMesh->myTriangles.push_back(Triangle());
		Triangle& triangle = navMesh.myNavMesh->myTriangles.back();
		triangle.myVetrices[0] = navMesh.myNavMesh->myVertices[face.x - 1];
		triangle.myVetrices[1] = navMesh.myNavMesh->myVertices[face.y - 1];
		triangle.myVetrices[2] = navMesh.myNavMesh->myVertices[face.z - 1];

		triangle.myIndices[0] = face.x - 1;
		triangle.myIndices[1] = face.y - 1;
		triangle.myIndices[2] = face.z - 1;

		triangle.myMiddlePos.x = (triangle.myVetrices[0].myPosition.x + triangle.myVetrices[1].myPosition.x + triangle.myVetrices[2].myPosition.x) / 3.0f;
		triangle.myMiddlePos.y = (triangle.myVetrices[0].myPosition.y + triangle.myVetrices[1].myPosition.y + triangle.myVetrices[2].myPosition.y) / 3.0f;
		triangle.myMiddlePos.z = (triangle.myVetrices[0].myPosition.z + triangle.myVetrices[1].myPosition.z + triangle.myVetrices[2].myPosition.z) / 3.0f;
	}


	for (Triangle& triangle : navMesh.myNavMesh->myTriangles)    //TODO: gör om skiten. Det är så oOptimiserat å äckligt. är även ful och äcklig. yuck
	{
		//std::array<Vertex, 3>& vertices = triangle.myVetrices;

		for (Triangle& otherTriangle : navMesh.myNavMesh->myTriangles)
		{
			if (triangle.myMiddlePos == otherTriangle.myMiddlePos) continue;

			bool wasAlreadyNeighbour = false;
			for (Triangle* triangleNeighbour : triangle.myNeighbours)
			{
				if (otherTriangle.myMiddlePos == triangleNeighbour->myMiddlePos)
				{
					wasAlreadyNeighbour = true;
					break;
				}
			}

			if (wasAlreadyNeighbour)
			{
				otherTriangle.myNeighbours.push_back(&triangle);
				continue;
			}


			int count = 0;
			for (Vertex& vertex : triangle.myVetrices)
			{
				for (Vertex& otherVertex : otherTriangle.myVetrices)
				{
					if (vertex.myPosition == otherVertex.myPosition)
					{
						count++;
					}
				}
			}
			if (count >= 2)
			{
				triangle.myNeighbours.push_back(&otherTriangle);
			}
		}
	}




	//navMesh.myNavMesh->myTriangles.erase(navMesh.myNavMesh->myTriangles.begin(), navMesh.myNavMesh->myTriangles.begin() + navMesh.myNavMesh->myTriangles.size() - 3);

	if (navMesh.myNavMesh->myTriangles.empty())
	{
		LOG_ERROR(LogType::Game) << "nav mesh failed to load";
		assert(false && "nav mesh failed to load");
	}

	return navMesh.myNavMesh;
}
