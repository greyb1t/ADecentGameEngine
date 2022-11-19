#pragma once
#include "Engine/GameObject/Components/Component.h"

class NavMeshThreaded;

namespace NavMesh
{
	class NavMesh;
}

namespace Engine
{
	class NavmeshComponent : public Component
	{
	public:
		COMPONENT_SINGLETON(NavmeshComponent, "NavmeshComponent");

		NavmeshComponent() = default;
		NavmeshComponent(GameObject* aGameObject);
		~NavmeshComponent();

		void Start() override;
		void Execute(eEngineOrder aOrder) override;

		NavMesh::NavMesh* GetNavmesh();
		NavMeshThreaded& GetNavThread();
		//NavMeshThreaded& GetNavThread2();

	private:
		NavMesh::NavMesh* myNavMesh = nullptr;
		NavMeshThreaded* myNavThread = nullptr;
		//NavMeshThreaded* myNavThread2 = nullptr;
	};
}
