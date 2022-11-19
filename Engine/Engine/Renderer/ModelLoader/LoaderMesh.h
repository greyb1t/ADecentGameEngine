#pragma once

#include "../Vertex.h"
#include "../VertexTypes.h"

namespace Engine
{
	class ModelTextureResource;
}

namespace Engine
{
	class LoaderModel;

	class LoaderMesh
	{
	public:
		LoaderMesh() = default;
		LoaderMesh(
			const aiMesh* aMesh,
			const aiNode* aNode,
			const aiVector3D aWorldPosition,
			const aiVector3D aWorldScale,
			const aiQuaternion aWorldRotation,
			LoaderModel& aLoaderModel,
			const int aVerticesStartIndex,
			const int aIndicesStartIndex);

		bool Init(const aiScene& aScene);
		void Init(const std::string& aName, const int aVertStart, const int aVertCount, const int aIndStart, const int aIndCount);

		void ReadBoneVertexData();

		const std::string& GetName() const;

		int GetVerticesCount() const;
		int GetVerticesStartIndex() const;

		int GetIndicesCount() const;
		int GetIndicesStartIndex() const;

		Vec3f GetWorldPosition() const;
		Vec3f GetWorldScale() const;
		Quatf GetWorldRotation() const;

		LoaderModel& GetLoaderModel();

		void Serialize(BinaryWriter& aBinaryStream) const
		{
			aBinaryStream.WriteString(myMeshName);
			aBinaryStream.WriteString(myAlbedoTexturePath);
			aBinaryStream.WriteString(myNormalTexturePath);
			aBinaryStream.WriteString(myMaterialTexturePath);

			aBinaryStream.Write<uint32_t>(myVerticesStartIndex);
			aBinaryStream.Write<uint32_t>(myVerticesCount);

			aBinaryStream.Write<uint32_t>(myIndicesStartIndex);
			aBinaryStream.Write<uint32_t>(myIndicesCount);
		}

		// TEMP USED BY CUSTOM BINARY FORMAT SERIALIZING ATM
		std::string myAlbedoTexturePath;
		std::string myNormalTexturePath;
		std::string myMaterialTexturePath;

		std::string myMaterialStem;
		std::string myMaterialPath;

	private:
		friend class LoaderModelBinary;
		friend class ModelTextureResource;
		friend class ModelTextureResource;

		D3D11_PRIMITIVE_TOPOLOGY DeterminePrimitiveTopology();

		void TryReadVerticesPositions();
		void TryReadIndices();
		void TryReadNormals();
		void TryReadTangents();
		void TryReadUVs();
		void TryReadBones();
		void TryReadColors();

	private:
		const aiMesh* myMesh = nullptr;
		const aiNode* myNode = nullptr;

		aiVector3D myWorldPosition;
		aiVector3D myWorldScale;
		aiQuaternion myWorldRotation;

		LoaderModel* myLoaderModel = nullptr;

		int myVerticesCount = 0;
		int myVerticesStartIndex = 0;

		int myIndicesCount = 0;
		int myIndicesStartIndex = 0;

		std::string myMeshName;


		// ANIMATION STUFF
		// CU::Matrix4f myGlobalInverseTransform;

		// Copy this data in modelloader to skinneddata and remove initforscene
		//std::vector<CU::Matrix4f> myBoneOffsets;
		//std::map<std::string, unsigned int> myBoneNameToIndex;

		std::vector<VertexBoneData> myCollectedBoneData;
	};
}