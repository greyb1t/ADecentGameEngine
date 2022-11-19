#pragma once

#include "..\RendererType.h"
#include "Engine\Renderer\Material\MeshMaterialInstance.h"
#include "Engine\Reflection\Reflectable.h"

namespace Engine
{
	class ModelComponent;
}

namespace Engine
{
	class ModelInstance;
	struct MeshData;
	class Mesh;
	class MeshMaterial;

	class MeshInstance : public Reflectable
	{
	public:
		void Init(ModelInstance* aModelInstance, const int aMeshIndex, const MaterialRef& aMaterialRef);

		ModelInstance* GetModelInstance() const { return myModelInstance; }
		int GetMeshIndex() const { return myMeshIndex; }

		void SetMaterial(const std::string& aMaterialName);
		void SetMaterial(const MaterialRef& aMaterialResource);

		MeshMaterialInstance& GetMaterialInstance();
		const MeshMaterialInstance& GetMaterialInstance() const;

		void SetCastShadows(const bool aCastShadows);
		bool IsCastingShadows() const;

		void Reflect(Reflector& aReflector) override;

	private:
		friend class ModelInstance;
		friend class ModelComponent;

		// NO NEED FOR THIS YET
		// TriangleMesh* myMesh = nullptr;

		ModelInstance* myModelInstance = nullptr;

		int myMeshIndex = 0;

		MeshMaterialInstance myMaterialInstance;

		bool myCastShadows = true;
	};
}