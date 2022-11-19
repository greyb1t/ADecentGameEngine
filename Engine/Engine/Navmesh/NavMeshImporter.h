#pragma once
#include "NavMesh.h"

namespace NavMesh
{
	class NavMeshImporter
	{
	private:
		struct NavigationMesh
		{
			NavMesh* myNavMesh;
			std::string myPath;
		};

	public:

		NavMesh* LoadNavMesh(std::string aFilePath);

	private:
		std::vector<NavigationMesh> myNavMeshes;
	};
}


