#pragma once

#include "NavMeshVertex.h"

namespace NavMesh
{
	class NavMesh;

	class NavMeshImporterBinary
	{
	public:
		struct BinaryNavmeshHeader
		{
			char myMagic[5] = {};

			uint16_t myMinorVersion = 0;
			uint16_t myMajorVersion = 0;

			uint32_t myVerticesCount = 0;
			uint32_t myVerticesFileOffset = 0;

			uint32_t myTriangleCount = 0;
			uint32_t myTriangleFileOffset = 0;
		};

		struct NavMeshVertex
		{
			float x = 0.f;
			float y = 0.f;
			float z = 0.f;
		};

		struct InternalNavMeshTriangle
		{
			NavMeshVertex myVertices[3] = { };
			uint32_t myIndices[3] = { };
			float myMiddlePos[3] = { };
			uint32_t myNeighboursCount = 0;
		};

		struct NavMeshTriangle
		{
			NavMeshVertex myVertices[3] = {};
			uint32_t myIndices[3] = {};
			float myMiddlePos[3] = {};
			uint32_t myNeighboursCount = 0;
			uint32_t myNeighbourIndices[1] = {};
		};

	public:
		bool LoadNavMesh(const std::string& aFilePath);

		NavMesh* CreateNavMesh(const CU::Vector3f& aOffset) const;

		const std::vector<const Vertex*> GetVertices() const;
		const std::vector<const NavMeshTriangle*> GetTriangles() const;

	private:
		bool ReadIntoBuffer(const std::string& aFilePath);

		void ReadVertices();
		void ReadTriangles();

	private:
		std::vector<uint8_t> myBuffer;

		const BinaryNavmeshHeader* myHeader = nullptr;

		std::vector<const Vertex*> myVertices;
		std::vector<const NavMeshTriangle*> myTriangles;
	};
}