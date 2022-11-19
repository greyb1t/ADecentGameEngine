#pragma once

#include "Engine\Renderer\Material\EffectPass.h"
#include "Engine\Renderer\Material\MeshMaterialInstance.h"
#include "Engine\Renderer\Shaders\ShaderConstants.h"

namespace Engine
{
	struct CulledMeshInstance;
	struct CulledMeshComponent;
	class RenderCommandList;
	class SpotLightComponent;
	class PointLightComponent;

	/*
	struct SingleMeshData
	{
		MeshMaterialInstance myMeshMaterialInstance;
		bool myCastShadows = true;
		Mat4f myTransform;
	};
	*/

	class Model;
	class Material;

	struct SingleMeshData2
	{
		SingleMeshData2() = default;
		SingleMeshData2(const CulledMeshInstance& aCulledMeshInstance);
		SingleMeshData2(const CulledMeshComponent& aCulledMeshComponent);

		// ModelRef myModel;
		Model* myModel;

		// TODO(filip): MaterialInstance is a pointer to game logic sidde
		// if ever threading in future, this is not thread safe as it can be destroyed
		// and modified by logic thread
		MaterialInstance* myMaterialInstance;
		// int test;

		Material* myMaterial;
		bool myCastShadows;
		Mat4f myTransform;
		int myMeshIndex;
		EffectPass* myPass;

		// TODO: When rendering on separarete threads,
		// I cannot use a pointer to the matrix array
		bool myHasSkeleton;
		const std::array<Mat4f, ShaderConstants::MaxBoneCount>* myAnimTransforms;
	};

	struct MeshSortKey
	{
		union
		{
			struct
			{
				uint64_t myInt64;
			};

			struct
			{
				// ORDER IS IMPORTANT
				uint16_t myMeshIndex;
				uint16_t myModelID;
				uint16_t myEffectPassID;
				uint16_t myMaterialID;

				/*
				//uint16_t myMaterialID;
				uint16_t myEffectPassID;
				uint16_t myModelID;
				uint16_t myMeshIndex;
				*/
			};
		};

		MeshSortKey()
		{
			myInt64 = 0;
		}
	};

	static_assert(sizeof(MeshSortKey) == sizeof(uint64_t));

	struct MeshCommandEntry
	{
		// TODO: When iterating to render these calls
		// use the previous SortKey to compare against.
		// if the soft key changed, something has changed

		MeshSortKey mySortKey;
		SingleMeshData2 myMeshData;

		inline bool operator() (
			const MeshCommandEntry& aLeft,
			const MeshCommandEntry& aRight)
		{
			return aLeft.mySortKey.myInt64 > aRight.mySortKey.myInt64;
		}
	};

	struct SpotlightCommandEntry
	{
		// TODO: change this, dont send pointer that is on logic thread
		SpotLightComponent* mySpotLight;

		std::vector<MeshCommandEntry> myShadowCasters;
	};

	struct PointLightCommandEntry
	{
		// TODO: change this, dont send pointer that is on logic thread
		PointLightComponent* myPointLight;

		// Each index is the side of the cube
		std::array<std::vector<MeshCommandEntry>, 6> myShadowCasters;
	};

	class RenderCommandList
	{
	public:
		std::vector<MeshCommandEntry> myMeshCommands;

		std::vector<SpotlightCommandEntry> mySpotlightCommands;
		std::vector<PointLightCommandEntry> myPointLightCommands;

		void Reset();

	private:
		friend class MeshCommandIterator;
	};
}