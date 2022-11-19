#include "pch.h"
#include "LoaderModel.h"
#include "../Animation/AssimpUtils.h"
#include "Engine/GameObject/GameObject.h"

namespace Engine
{
	LoaderModel::LoaderModel(Assimp::Importer* aImporter)
		: myImporter(aImporter)
	{
	}

	bool LoaderModel::Init(const Path& aModelPath, GameObject* aParent)
	{
		myPath = aModelPath.ToString();

		unsigned int flags = aiProcessPreset_TargetRealtime_MaxQuality |
			aiProcess_MakeLeftHanded |
			aiProcess_FlipWindingOrder |
			aiProcess_FlipUVs |
			aiProcess_Triangulate |
			aiProcess_CalcTangentSpace |
			// aiProcess_JoinIdenticalVertices |
			aiProcess_SortByPType;

		// Having this line assert becuse to many bones found
		flags &= ~aiProcess_JoinIdenticalVertices;

		// Due to using aiProcess_Triangulate, we can assume that each face only has 3 vertices
		myScene = myImporter->ReadFile(aModelPath.ToString().c_str(), flags);

		if (myScene == nullptr)
		{
			LOG_ERROR(LogType::Renderer) << "Assimp failed to load model " << aModelPath;
			return false;
		}

		//if (myScene->mNumMeshes == 0)
		//{
		//	return false;
		//}

		auto temp = myScene->mRootNode->mTransformation;
		temp.Inverse();
		myGlobalInverseTransform = Mat4f::Transpose(AiMatrixToCUMatrix(temp));

		// NOTE(filip): not using because we get the incorrect result I think, not sure
		// secondly, we won't need this either 
		CreateMeshes(myScene->mRootNode, {}, aiVector3D(1, 1, 1), {});

		if (aParent)
		{
			CreateGameObjectHierarchy(aParent, myScene->mRootNode);
			return true;
		}

		CreateNodeHierarchy(-1, myScene->mRootNode);

		/*for (int nodeIndex = 0; nodeIndex < myScene->mRootNode->mNumChildren; ++nodeIndex)
		{
			const aiNode* node = myScene->mRoot
			Node->mChildren[nodeIndex];

			for (int meshIndex = 0; meshIndex < node->mNumMeshes; ++meshIndex)
			{
				const unsigned int indexIntoMeshes = node->mMeshes[meshIndex];

				LoaderMesh loaderMesh(
					myScene->mMeshes[indexIntoMeshes],
					node,
					*this,
					myVertices.size(),
					myIndices.size());

				if (!loaderMesh.Start(*myScene))
				{
					LOG_ERROR(LogType::Renderer) << "Failed to read assimp mesh";
					return false;
				}

				myMeshes.push_back(loaderMesh);
			}
		}*/

		//for (int i = 0; i < static_cast<int>(myScene->mNumMeshes); ++i)
		//{
		//	LoaderMesh loaderMesh(
		//		myScene->mMeshes[i],
		//		myScene->mRootNode->mChildren[i],
		//		{}, {}, {},
		//		*this,
		//		myVertices.size(),
		//		myIndices.size());

		//	if (!loaderMesh.Init(*myScene))
		//	{
		//		LOG_ERROR(LogType::Renderer) << "Failed to read assimp mesh";
		//		return false;
		//	}

		//	myMeshes.push_back(loaderMesh);
		//}

		return true;
	}

	void LoaderModel::Init(const std::vector<DefaultVertex>& someVertices, const std::vector<unsigned int>& someIndices)
	{
		myVertices = someVertices;
		myIndices = someIndices;

		auto mesh = LoaderMesh();
		mesh.Init("mesh", 0, myVertices.size(), 0, myIndices.size());
		myMeshes.emplace_back(mesh);
	}

	const std::vector<LoaderMesh>& LoaderModel::GetMeshes() const
	{
		return myMeshes;
	}

	std::vector<LoaderMesh>& LoaderModel::GetMeshes()
	{
		return myMeshes;
	}

	const std::string& LoaderModel::GetPath() const
	{
		return myPath;
	}

	const std::vector<DefaultVertex>& LoaderModel::GetVertices() const
	{
		return myVertices;
	}

	const std::vector<unsigned int>& LoaderModel::GetIndices() const
	{
		return myIndices;
	}

	bool LoaderModel::CreateMeshes(
		const aiNode* aNode,
		const aiVector3D aWorldPosition,
		const aiVector3D aWorldScale,
		const aiQuaternion aWorldRotation)
	{
		for (int meshIndex = 0; meshIndex < aNode->mNumMeshes; ++meshIndex)
		{
			const unsigned int indexIntoMeshes = aNode->mMeshes[meshIndex];

			LoaderMesh loaderMesh(
				myScene->mMeshes[indexIntoMeshes],
				aNode,
				aWorldPosition,
				aWorldScale,
				aWorldRotation,
				*this,
				myVertices.size(),
				myIndices.size());

			if (!loaderMesh.Init(*myScene))
			{
				LOG_ERROR(LogType::Renderer) << "Failed to read assimp mesh";
				return false;
			}
			
			loaderMesh.myMaterialStem = myScene->mMaterials[myScene->mMeshes[indexIntoMeshes]->mMaterialIndex]->GetName().C_Str();

			myMeshes.push_back(loaderMesh);
		}

		for (int nodeIndex = 0; nodeIndex < aNode->mNumChildren; ++nodeIndex)
		{
			const aiNode* node = aNode->mChildren[nodeIndex];

			aiVector3D scaling;
			aiQuaternion rotation;
			aiVector3D trans;

			node->mTransformation.Decompose(scaling, rotation, trans);

			// NOTE(filip): we must multiply the transforms in opposite order 
			// because in assimp they are not in row major
			if (!CreateMeshes(
				node,
				aWorldPosition + trans,
				aiVector3D(aWorldScale.x * scaling.x, aWorldScale.y * scaling.y, aWorldScale.z * scaling.z),
				aWorldRotation * rotation))
			{
				return false;
			}
		}

		return true;
	}

	void LoaderModel::CreateGameObjectHierarchy(
		GameObject* aParent,
		const aiNode* aNode)
	{
		aiVector3D scaling;
		aiQuaternion rotation;
		aiVector3D trans;

		aNode->mTransformation.Decompose(scaling, rotation, trans);

		auto g = aParent->GetScene()->AddGameObject<GameObject>();
		g->GetTransform().SetParent(&aParent->GetTransform());
		g->GetTransform().SetScaleLocal(Vec3f(scaling.x, scaling.y, scaling.z));
		g->GetTransform().SetPositionLocal(Vec3f(trans.x, trans.y, trans.z));
		g->GetTransform().SetRotationLocal(AiQuaternionToCUQuaternion(rotation));
		g->SetName(aNode->mName.C_Str());

		assert(aNode->mNumMeshes <= 1 && "a node cannot have more than 1 mesh");

		// auto meshComponent = g->AddComponent<Engine::MeshComponent>();

		for (int meshIndex = 0; meshIndex < aNode->mNumMeshes; ++meshIndex)
		{
			const unsigned int indexIntoMeshes = aNode->mMeshes[meshIndex];

			LoaderMesh loaderMesh(
				myScene->mMeshes[indexIntoMeshes],
				aNode,
				{},
				{},
				{},
				*this,
				myVertices.size(),
				myIndices.size());

			if (!loaderMesh.Init(*myScene))
			{
				LOG_ERROR(LogType::Renderer) << "Failed to read assimp mesh";
				assert(false);
			}
		}

		for (int nodeIndex = 0; nodeIndex < aNode->mNumChildren; ++nodeIndex)
		{
			const aiNode* node = aNode->mChildren[nodeIndex];

			// NOTE(filip): we must multiply the transforms in opposite order 
			// because in assimp they are not in row major
			CreateGameObjectHierarchy(g, node);
		}
	}

	void LoaderModel::CreateNodeHierarchy(const int aParentIndex, const aiNode* aNode)
	{
		aiVector3D scaling;
		aiQuaternion rotation;
		aiVector3D trans;

		aNode->mTransformation.Decompose(scaling, rotation, trans);

		// If a node has more any meshes, we NEED to create a new node for each mesh
		// because our rendering should only support one MeshComponent on one GameObject
		// Otherwise, we create ONE node without mesh ONLY because we still
		// need to keep the transform hierarchy
		//if (aNode->mNumMeshes > 0)
		//{
		//	for (int meshIndex = 0; meshIndex < aNode->mNumMeshes; ++meshIndex)
		//	{
		//		Node theNode;

		//		const unsigned int indexIntoMeshes = aNode->mMeshes[meshIndex];

		//		theNode.myMeshIndex = indexIntoMeshes;

		//		theNode.myTransform.SetScaleLocal(Vec3f(scaling.x, scaling.y, scaling.z));
		//		theNode.myTransform.SetPositionLocal(Vec3f(trans.x, trans.y, trans.z));
		//		theNode.myTransform.SetRotationLocal(AiQuaternionToCUQuaternion(rotation));

		//		theNode.myParentIndex = aParentIndex;

		//		if (aParentIndex != -1)
		//		{
		//			myNodes[aParentIndex].myChildrenIndices.push_back(myNodes.size());
		//		}

		//		theNode.myName = aNode->mName.C_Str();

		//		myNodes.push_back(theNode);
		//	}
		//}
		// else
		{
			Node theNode;

			for (int i = 0; i < aNode->mNumMeshes; ++i)
			{
				const unsigned int indexIntoMeshes = aNode->mMeshes[i];
				theNode.myMeshIndices.push_back(indexIntoMeshes);
			}

			theNode.myTransform.SetScaleLocal(Vec3f(scaling.x, scaling.y, scaling.z));
			theNode.myTransform.SetPositionLocal(Vec3f(trans.x, trans.y, trans.z));
			theNode.myTransform.SetRotationLocal(AiQuaternionToCUQuaternion(rotation));

			theNode.myParentIndex = aParentIndex;

			if (aParentIndex != -1)
			{
				myNodes[aParentIndex].myChildrenIndices.push_back(myNodes.size());
			}

			theNode.myName = aNode->mName.C_Str();

			myNodes.push_back(theNode);
		}

		const int parentindex = static_cast<int>(myNodes.size()) - 1;

		for (int nodeIndex = 0; nodeIndex < aNode->mNumChildren; ++nodeIndex)
		{
			const aiNode* node = aNode->mChildren[nodeIndex];

			// NOTE(filip): we must multiply the transforms in opposite order 
			// because in assimp they are not in row major
			CreateNodeHierarchy(parentindex, node);
		}
	}
}
