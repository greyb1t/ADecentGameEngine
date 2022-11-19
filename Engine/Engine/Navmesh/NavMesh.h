#pragma once
#include <thread>
#include <mutex>

#include "NavMeshVertex.h"

namespace Engine
{
class Scene;
}

namespace NavMesh
{
	struct Triangle;

	struct TriangleInfo
	{
		Triangle* myTriangle = nullptr;
		float myCurrentDistance = FLT_MAX;
		TriangleInfo* myPredecessor = nullptr;
		unsigned int myIndex = 0;

		friend bool operator<(const TriangleInfo& aInfo1, const TriangleInfo& aInfo2)
		{
			return aInfo1.myCurrentDistance > aInfo2.myCurrentDistance;
		}

		friend bool operator==(const TriangleInfo& aInfo1, const TriangleInfo& aInfo2)
		{
			return aInfo1.myCurrentDistance == aInfo2.myCurrentDistance;
		}
	};

	struct Triangle
	{
		std::array<Vertex, 3> myVetrices;
		unsigned int myIndices[3];
		CU::Vector3f myMiddlePos;
		std::vector<Triangle*> myNeighbours;
		TriangleInfo* myInfo = nullptr;
	};


	class NavMesh
	{
	public:
		NavMesh();
		~NavMesh();

		void HeapTest();

		//Gives a vector of positions. The first one is in the back
		std::vector<CU::Vector3f> Pathfind(const CU::Vector3f& aStartPos, const CU::Vector3f& aEndPos);

		bool IsPositionOnNavMesh(const CU::Vector3f& aPos);
		bool ArePositionsOnNavMesh(const CU::Vector3f& aStartPos, const CU::Vector3f& aEndPos);
		bool ArePositionsSameTriangle(const CU::Vector3f& aFirstPos, const CU::Vector3f& aSecondPos);

		void GetClosestTrianglePos(const CU::Vector3f& aPos, CU::Vector3f& aOutPos);
		Triangle* GetClosestTriangle(const CU::Vector3f& aPos);

		void DrawMesh();

		const std::vector<Vertex>& GetVertices() const;
		const std::vector<Triangle>& GetTriangles() const;
		Triangle* IsInside(const CU::Vector3f& aPosition); // moved to public field by Jesper

		CU::Vector3f GetRandomPosition();
	private:
		friend class NavMeshImporter;
		friend class NavMeshImporterBinary;

		void Init(const std::vector<Triangle>& aTriangleList);


		float GetTriangleArea(const CU::Vector3f& aFirstPos, const CU::Vector3f& aSecondPos, const CU::Vector3f& aThirdPos) const;

		std::vector<CU::Vector3f> AStar(const Triangle& aStartTriangle, const Triangle& aEndTriangle);
		float HeuristicCalculation(const CU::Vector3f& aStartPos, const CU::Vector3f& aEndPos) const;

		void StringPull(std::vector<CU::Vector3f>& aOutList);
		bool CanStringPull(const CU::Vector3f& aFirstPos, const CU::Vector3f& aSecondPos); //Gör om skiten, nuvaramde legit cuttar corners i sin pathfinding

		std::vector<Vertex> myVertices;
		std::vector<Triangle> myTriangles;

		unsigned int myNumberOfTriangles = 0;
		unsigned int myNumberOfVertices = 0;
	};
}
