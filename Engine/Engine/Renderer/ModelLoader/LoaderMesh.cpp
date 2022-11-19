#include "pch.h"
#include "LoaderMesh.h"
#include "LoaderModel.h"
#include "../Animation/AssimpUtils.h"

namespace Engine
{
	LoaderMesh::LoaderMesh(
		const aiMesh* aMesh,
		const aiNode* aNode,
		const aiVector3D aWorldPosition,
		const aiVector3D aWorldScale,
		const aiQuaternion aWorldRotation,
		LoaderModel& aLoaderModel,
		const int aVerticesStartIndex,
		const int aIndicesStartIndex)
		: myMesh(aMesh),
		myNode(aNode),
		myWorldPosition(aWorldPosition),
		myWorldRotation(aWorldRotation),
		myWorldScale(aWorldScale),
		myLoaderModel(&aLoaderModel),
		myVerticesStartIndex(aVerticesStartIndex),
		myIndicesStartIndex(aIndicesStartIndex)
	{
	}

	bool LoaderMesh::Init(const aiScene& aScene)
	{
		DeterminePrimitiveTopology();

		// NOT WORKING
		// myLocalPosition = Vec3f(myWorldPosition.x, myWorldPosition.y, myWorldPosition.z);
		// myLocalScale = Vec3f(myWorldScale.x, myWorldScale.y, myWorldScale.z);
		// myLocalRotation = AiQuaternionToCUQuaternion(myWorldRotation);


		myMeshName = myMesh->mName.C_Str();

		//auto dikc = aScene.mMaterials[myMesh->mMaterialIndex]->GetName();
		//int tes1t = aScene.mMaterials[myMesh->mMaterialIndex]->GetTextureCount(aiTextureType_BASE_COLOR);
		//int tes2 = aScene.mMaterials[myMesh->mMaterialIndex]->GetTextureCount(aiTextureType_DIFFUSE);
		//int test3 = aScene.mMaterials[myMesh->mMaterialIndex]->GetTextureCount(aiTextureType_NONE);

		//aiString texture_file;
		//aScene.mMaterials[myMesh->mMaterialIndex]->Get(AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE, 0), texture_file);

		TryReadVerticesPositions();
		TryReadIndices();
		TryReadNormals();
		TryReadTangents();
		TryReadUVs();
		TryReadBones();
		TryReadColors();

		return true;
	}

	void LoaderMesh::Init(const std::string& aName, const int aVertStart, const int aVertCount, const int aIndStart, const int aIndCount)
	{
		myMeshName = aName;
		myVerticesStartIndex = aVertStart;
		myVerticesCount = aVertCount;
		myIndicesStartIndex = aIndStart;
		myIndicesCount = aIndCount;
	}

	// const std::vector<DefaultVertex>& LoaderMesh::GetVertices() const
	// {
	// 	return myVertices;
	// }
	// 
	// const std::vector<unsigned int>& LoaderMesh::GetIndices() const
	// {
	// 	return myIndices;
	// }

	void LoaderMesh::ReadBoneVertexData()
	{
		if (myMesh->HasBones())
		{
			myCollectedBoneData.resize(myMesh->mNumVertices);
			myLoaderModel->myBoneOffsets.resize(myLoaderModel->myBoneOffsets.size() + myMesh->mNumBones);

			for (unsigned int i = 0; i < myMesh->mNumBones; i++)
			{
				std::string boneName = myMesh->mBones[i]->mName.data;

				// Remove the namespace part of the meshname for meshes with namespaces
				const auto colonIndex = boneName.find(':');

				if (colonIndex != std::string::npos)
				{
					boneName = boneName.substr(colonIndex + 1);
				}

				const bool notFound = myLoaderModel->myBoneNameToIndex.find(boneName) == myLoaderModel->myBoneNameToIndex.end();

				unsigned int boneIndex = 0;

				if (notFound)
				{
					boneIndex = myLoaderModel->myBonesCount;
					myLoaderModel->myBonesCount++;

					myLoaderModel->myBoneOffsets[boneIndex] = Mat4f::Transpose(AiMatrixToCUMatrix(myMesh->mBones[i]->mOffsetMatrix));
					myLoaderModel->myBoneNameToIndex[boneName] = boneIndex;
				}
				else
				{
					boneIndex = myLoaderModel->myBoneNameToIndex[boneName];
				}

				for (unsigned int j = 0; j < myMesh->mBones[i]->mNumWeights; j++)
				{
					const unsigned int vertexID = myMesh->mBones[i]->mWeights[j].mVertexId;
					const float weight = myMesh->mBones[i]->mWeights[j].mWeight;

					myCollectedBoneData[vertexID].AddBoneData(boneIndex, weight);
				}

				int test = 0;
			}
		}
	}

	const std::string& LoaderMesh::GetName() const
	{
		return myMeshName;
	}

	int LoaderMesh::GetVerticesCount() const
	{
		return myVerticesCount;
	}

	int LoaderMesh::GetVerticesStartIndex() const
	{
		return myVerticesStartIndex;
	}

	int LoaderMesh::GetIndicesCount() const
	{
		return myIndicesCount;
	}

	int LoaderMesh::GetIndicesStartIndex() const
	{
		return myIndicesStartIndex;
	}

	Vec3f LoaderMesh::GetWorldPosition() const
	{
		return Vec3f(myWorldPosition.x, myWorldPosition.y, myWorldPosition.z);
	}

	Vec3f LoaderMesh::GetWorldScale() const
	{
		return Vec3f(myWorldScale.x, myWorldScale.y, myWorldScale.z);
	}

	Quatf LoaderMesh::GetWorldRotation() const
	{
		return AiQuaternionToCUQuaternion(myWorldRotation);
	}

	Engine::LoaderModel& LoaderMesh::GetLoaderModel()
	{
		return *myLoaderModel;
	}

	D3D11_PRIMITIVE_TOPOLOGY LoaderMesh::DeterminePrimitiveTopology()
	{
		// TODO: Implement this if needed
		return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	}

	void LoaderMesh::TryReadVerticesPositions()
	{
		// myVertices.clear();

		if (!myMesh->HasPositions())
		{
			return;
		}

		const auto vertices = myMesh->mVertices;
		const int vertexCount = static_cast<int>(myMesh->mNumVertices);

		myVerticesCount = vertexCount;

		// myVertices.reserve(vertexCount);

		for (int i = 0; i < vertexCount; ++i)
		{
			// TODO: Will we use multiple different Vertex 
			// structures in the future, do I need to support this?

			DefaultVertex resultVertex;

			resultVertex.myPosition = C::Vector4f(vertices[i].x, vertices[i].y, vertices[i].z, 1.f);

			myLoaderModel->myVertices.push_back(resultVertex);
		}
	}

	void LoaderMesh::TryReadIndices()
	{
		// myIndices.clear();

		assert(myMesh->mFaces != nullptr && "cannot be null");

		// TODO: if using aiProcess_Triangulate, I can reserve in the vector by mNumFace * 3
		// should I do it or not?
		// Remember that line and point does not triangulate

		int indicesCount = 0;

		for (int faceIndex = 0; faceIndex < static_cast<int>(myMesh->mNumFaces); ++faceIndex)
		{
			const auto& face = myMesh->mFaces[faceIndex];

			for (int indicesIndex = 0; indicesIndex < static_cast<int>(face.mNumIndices); ++indicesIndex)
			{
				myLoaderModel->myIndices.push_back(face.mIndices[indicesIndex]);
				++indicesCount;
			}
		}

		myIndicesCount = indicesCount;
	}

	void LoaderMesh::TryReadNormals()
	{
		if (!myMesh->HasNormals())
		{
			return;
		}

		for (int i = 0; i < static_cast<int>(myMesh->mNumVertices); ++i)
		{
			// W = 0, normals are also diretions
			myLoaderModel->myVertices[myVerticesStartIndex + i].myNormal = C::Vector4f(myMesh->mNormals[i].x, myMesh->mNormals[i].y, myMesh->mNormals[i].z, 0.f);
		}
	}

	void LoaderMesh::TryReadTangents()
	{
		if (!myMesh->HasTangentsAndBitangents())
		{
			return;
		}

		assert(myMesh->mTangents && myMesh->mBitangents);

		for (int i = 0; i < static_cast<int>(myMesh->mNumVertices); ++i)
		{
			// W = 0 because it is direction?? or doesn't matter bcuz we scrap in the shader either way?
			myLoaderModel->myVertices[myVerticesStartIndex + i].myTangent = C::Vector4f(myMesh->mTangents[i].x, myMesh->mTangents[i].y, myMesh->mTangents[i].z, 0.f);
			myLoaderModel->myVertices[myVerticesStartIndex + i].myBinormal = C::Vector4f(myMesh->mBitangents[i].x, myMesh->mBitangents[i].y, myMesh->mBitangents[i].z, 0.f);
		}
	}

	void LoaderMesh::TryReadUVs()
	{
		// Give warning if the mesh have more texture coords that we support
		for (int i = 1; i < AI_MAX_NUMBER_OF_TEXTURECOORDS; ++i)
		{
			if (myMesh->HasTextureCoords(i))
			{
				LOG_ERROR(LogType::Renderer) << "Mesh has more texture coords than we support " << myLoaderModel->myPath;
				break;
			}
		}

		// I dont know what multiple texture sets means, so only one supported for now
		if (myMesh->HasTextureCoords(0))
		{
			for (int i = 0; i < static_cast<int>(myMesh->mNumVertices); ++i)
			{
				myLoaderModel->myVertices[myVerticesStartIndex + i].myUVs = Vec2f(
					myMesh->mTextureCoords[0][i].x,
					myMesh->mTextureCoords[0][i].y);
			}
		}
	}

	void LoaderMesh::TryReadBones()
	{
		if (myMesh->HasBones())
		{
			ReadBoneVertexData();

			for (int i = 0; i < static_cast<int>(myMesh->mNumVertices); ++i)
			{
				myLoaderModel->myVertices[myVerticesStartIndex + i].myBones = C::Vector4f(
					static_cast<float>(myCollectedBoneData[i].myIDs[0]),
					static_cast<float>(myCollectedBoneData[i].myIDs[1]),
					static_cast<float>(myCollectedBoneData[i].myIDs[2]),
					static_cast<float>(myCollectedBoneData[i].myIDs[3]));

				myLoaderModel->myVertices[myVerticesStartIndex + i].myWeights = C::Vector4f(
					static_cast<float>(myCollectedBoneData[i].myWeights[0]),
					static_cast<float>(myCollectedBoneData[i].myWeights[1]),
					static_cast<float>(myCollectedBoneData[i].myWeights[2]),
					static_cast<float>(myCollectedBoneData[i].myWeights[3]));
			}
		}
	}

	void LoaderMesh::TryReadColors()
	{
		if (!myMesh->HasVertexColors(0))
		{
			return;
		}

		for (int i = 0; i < static_cast<int>(myMesh->mNumVertices); ++i)
		{
			// W = 0, normals are also diretions
			myLoaderModel->myVertices[myVerticesStartIndex + i].myColor =
				C::Vector4f(
					myMesh->mColors[0][i].r,
					myMesh->mColors[0][i].g,
					myMesh->mColors[0][i].b,
					myMesh->mColors[0][i].a);
		}
	}

}