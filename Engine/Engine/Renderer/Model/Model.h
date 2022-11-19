#pragma once

#include "Engine\ResourceManagement\ResourceRef.h"
#include "Engine/Renderer/Animation/OurNode.h"
#include "Engine/LiteTransform.h"
#include "Engine\Renderer\Vertex.h"

namespace Engine
{
	class Material;

	struct MeshData
	{
		int myVertexCount = 0;
		int myIndicesCount = 0;

		int myStartIndexLocation = 0;
		int myBaseVertexLocation = 0;

		std::array<TextureRef, 3> myTexture = {};
		MaterialRef myDefaultMaterial;

		std::string myMeshName;

		LiteTransform myTransform;
	};

	struct FbxNode
	{
		std::string myName;
		LiteTransform myTransform;
		std::vector<int> myMeshIndices;
		int myParentNodeIndex = -1;
		std::vector<int> myChildrenIndices;
	};

	class Model
	{
	public:
		Model();
		~Model();

		ID3D11Buffer* GetIndexBuffer();
		ID3D11Buffer* GetVertexBuffer();
		ID3D11Buffer** GetVertexBufferPtr();

		UINT* GetStride();
		UINT* GetOffset();

		const std::vector<MeshData>& GetMeshDatas() const;
		std::vector<MeshData>& GetMeshDatas();

		// animations
		const std::map<std::string, unsigned int>& GetBoneNameToIndex() const { return myBoneNameToIndex; }
		const std::vector<Mat4f>& GetBoneOffsets() const { return myBoneOffsets; }
		unsigned int GetBonesCount() const { return myBonesCount; }
		const Mat4f& GetGlobalInverseTransform() const { return myGlobalInverseTransform; }

		const C::Sphere<float>& GetBoundingSphere() const;

		bool IsSkinned() const
		{
			return GetBonesCount() > 0;
		}

		const std::vector<FbxNode>& GetNodes() const { return myNodes; }

		// TODO: change to private, public for now
	public:
		std::vector<MeshData> myMeshDatas;

		D3D11_PRIMITIVE_TOPOLOGY myPrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;

		UINT myStride = 0;
		UINT myOffset = 0;

		ComPtr<ID3D11Buffer> myIndexBuffer = nullptr;
		ComPtr<ID3D11Buffer> myVertexBuffer = nullptr;

		C::Sphere<float> myLocalBoundingSphere;

		// gives parent index of ith index
		// index is into myNodes
		std::vector<FbxNode> myNodes;

		// ANIMATIONS STUFF
		Mat4f myGlobalInverseTransform;

		// Copy this data in modelloader to skinneddata and remove initforscene
		std::vector<Mat4f> myBoneOffsets;
		std::map<std::string, unsigned int> myBoneNameToIndex;

		std::vector<VertexBoneData> myCollectedBoneData;

		unsigned int myBonesCount = 0;
	};
}
