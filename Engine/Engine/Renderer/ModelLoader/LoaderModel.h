#pragma once

#include "LoaderMesh.h"
#include "..\Animation\OurNode.h"
#include "Engine/LiteTransform.h"

class GameObject;

namespace Assimp
{
	class Importer;
}

namespace Engine
{
	class ModelImportState;
	class ModelTextureResource;
}

namespace Engine
{
	class LoaderModel
	{
	public:
		struct Node
		{
			std::string myName;
			LiteTransform myTransform;
			int myParentIndex = -1;
			std::vector<int> myChildrenIndices;
			std::vector<int> myMeshIndices;
			// int myMeshIndex = -1;
		};

		LoaderModel() = default;
		LoaderModel(Assimp::Importer* aImporter);

		bool Init(const Path& aModelPath, GameObject* aParent = nullptr);

		void Init(const std::vector<DefaultVertex>& someVertices, const std::vector<unsigned int>& someIndices);

		const std::vector<LoaderMesh>& GetMeshes() const;
		std::vector<LoaderMesh>& GetMeshes();

		const std::map<std::string, unsigned int>& GetBoneNameToIndex() const { return myBoneNameToIndex; }
		const std::vector<Mat4f>& GetBoneOffsets() const { return myBoneOffsets; }
		unsigned int GetBonesCount() const { return myBonesCount; }
		const Mat4f& GetGlobalInverseTransform() const { return myGlobalInverseTransform; }

		const std::string& GetPath() const;

		const std::vector<DefaultVertex>& GetVertices() const;
		const std::vector<unsigned int>& GetIndices() const;

		const std::vector<Node>& GetNodes() const { return myNodes; }

	private:
		friend class LoaderMesh;
		friend class LoaderModelBinary;
		friend class ModelCreationState;
		friend class ModelTextureResource;

		friend class ModelImportState;
		friend class ModelTextureResource;

		bool CreateMeshes(
			const aiNode* aNode, 
			const aiVector3D aWorldPosition,
			const aiVector3D aWorldScale,
			const aiQuaternion aWorldRotation);

		void CreateGameObjectHierarchy(
			GameObject* aParent,
			const aiNode* aNode);

		void CreateNodeHierarchy(
			const int aParentIndex,
			const aiNode* aNode);

	private:
		Assimp::Importer* myImporter = nullptr;
		const aiScene* myScene = nullptr;

		// for debuggin
		std::string myPath;

		std::vector<LoaderMesh> myMeshes;

		// gives parent index of ith index
		// index is into myNodes
		std::vector<Node> myNodes;

		// Since we're creating ONE single d3d11 buffer for 
		// all vertices and indices, we store vertices and indices 
		// in here instead of the meshes
		std::vector<DefaultVertex> myVertices;
		std::vector<unsigned int> myIndices;

		// ANIMATION STUFF
		Mat4f myGlobalInverseTransform;

		// Copy this data in modelloader to skinneddata and remove initforscene
		std::vector<Mat4f> myBoneOffsets;
		std::map<std::string, unsigned int> myBoneNameToIndex;

		unsigned int myBonesCount = 0;
	};
}