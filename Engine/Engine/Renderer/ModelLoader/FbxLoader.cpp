#include "pch.h"
#include "FbxLoader.h"
#include "LoaderModel.h"
#include "Engine\Renderer\Model\Model.h"
#include "..\ResourceManagement\ResourceRef.h"
#include "..\ResourceManagement\Resources\TextureResource.h"
#include "../ResourceManagement/ResourceThreadContext.h"
#include "..\ResourceManagement\ResourceManager.h"
#include "Binary\LoaderModelBinary.h"
#include "..\Texture\TextureLoader.h"
#include "Binary/model_generated.h"
#include "Engine\Editor\FileTypes.h"
#include "Engine\ResourceManagement\Resources\ModelResource.h"
#include "Engine\Paths.h"
#include "Engine\Engine.h"
#include "Engine\Renderer\GraphicsEngine.h"
#include "Engine\Renderer\Material\MaterialFactory.h"
#include "Engine\ResourceManagement\Resources\MaterialResource.h"

Engine::FbxLoader::FbxLoader(ID3D11Device* aDevice, ResourceManager& aResourceManagerHandler)
	: myDevice(aDevice),
	myResourceManagerHandler(aResourceManagerHandler)
{
}

bool Engine::FbxLoader::Init()
{
	myImporter = MakeOwned<Assimp::Importer>();

	return true;
}

Owned<Engine::Model> Engine::FbxLoader::LoadFbx(Path aFbxPath, ModelResource& aFbxResource)
{
	// std::replace(aFbxPath.begin(), aFbxPath.end(), '/', '\\');

	// fuck the FBX format bruh
	// aFbxPath.ChangeExtension(".model");

#if 1
	if (aFbxPath.IsExtension(FileType::Extension_Model))
	{
		//return LoadCustomModel(Path(aFbxPath));
		return LoadCustomModel2(Path(aFbxPath), aFbxResource);
	}

	if (aFbxPath.IsExtension(".model2"))
	{
		return LoadCustomModel2(Path(aFbxPath), aFbxResource);
	}

	LOG_ERROR(LogType::Resource) << "FBX NOT SUPPORTED ANYMORE: " << aFbxPath;
#else
	aFbxPath.ChangeExtension(".fbx");
#endif

	// return nullptr;

	LoaderModel loaderModel(myImporter.get());

	if (!loaderModel.Init(aFbxPath))
	{
		LOG_ERROR(LogType::Renderer) << "Failed to parse the model from assimp " << aFbxPath;

		// Loader model placeholder
		return LoadCustomModel2(Path("Assets\\Engine\\Placeholders\\Cube\\EN_G_Cube_01.model"), aFbxResource);

		// return nullptr;
	}

	// std::vector<DefaultVertex> combinedVertices;
	// std::vector<unsigned int> combinedIndices;

	std::vector<MeshData> resultMeshDatas;

	const auto& meshes = loaderModel.GetMeshes();

	const bool singleMesh = meshes.size() == 1;

	for (int meshIndex = 0; meshIndex < meshes.size(); ++meshIndex)
	{
		const auto& loaderMesh = meshes[meshIndex];

		// const auto& vertices = loaderMesh.GetVertices();
		// const auto& indices = loaderMesh.GetIndices();

		MeshData meshData = {};
		{
			meshData.myMeshName = loaderMesh.GetName();

			// Remove the namespace part of the meshname for meshes with namespaces
			const auto colonIndex = meshData.myMeshName.find(':');

			if (colonIndex != std::string::npos)
			{
				meshData.myMeshName = meshData.myMeshName.substr(colonIndex + 1);
			}

			meshData.myVertexCount = loaderMesh.GetVerticesCount();
			meshData.myIndicesCount = loaderMesh.GetIndicesCount();

			meshData.myBaseVertexLocation = loaderMesh.GetVerticesStartIndex();
			meshData.myStartIndexLocation = loaderMesh.GetIndicesStartIndex();

			//meshData.myStartIndexLocation = std::max(static_cast<int>(combinedIndices.size()), 0);
			//meshData.myBaseVertexLocation = std::max(static_cast<int>(combinedVertices.size()), 0);

			std::wstring texturePath = aFbxPath.ToWString();
			{
				texturePath.erase(texturePath.end() - 4, texturePath.end());
				texturePath.append(L"_C.dds");

				if (!singleMesh)
				{
					texturePath.insert(texturePath.find_last_of('\\') + 1, std::to_wstring(meshIndex) + L"_");
				}
			}

			TextureRef albedoTextureRes =
				myResourceManagerHandler.CreateRef<TextureResource>(
					std::string(texturePath.begin(), texturePath.end()));
			albedoTextureRes->Load();
			aFbxResource.AddChild(albedoTextureRes);

			// temp awy to get normal texture
			std::wstring normalPath = aFbxPath.ToWString();
			{
				normalPath.erase(normalPath.end() - 4, normalPath.end());
				normalPath.append(L"_N.dds");

				if (!singleMesh)
				{
					normalPath.insert(normalPath.find_last_of('\\') + 1, std::to_wstring(meshIndex) + L"_");
				}
			}

			TextureRef normalTextureRes =
				myResourceManagerHandler.CreateRef<TextureResource>(
					std::string(normalPath.begin(), normalPath.end()));
			normalTextureRes->Load();
			aFbxResource.AddChild(normalTextureRes);

			// temp awy to get normal texture
			std::wstring materialPath = aFbxPath.ToWString();
			{
				materialPath.erase(materialPath.end() - 4, materialPath.end());
				materialPath.append(L"_M.dds");

				if (!singleMesh)
				{
					materialPath.insert(materialPath.find_last_of('\\') + 1, std::to_wstring(meshIndex) + L"_");
				}
			}

			TextureRef materialTextureRes =
				myResourceManagerHandler.CreateRef<TextureResource>(
					std::string(materialPath.begin(), materialPath.end()));
			materialTextureRes->Load();
			aFbxResource.AddChild(materialTextureRes);

			meshData.myTexture[0] = albedoTextureRes;
			meshData.myTexture[1] = normalTextureRes;
			meshData.myTexture[2] = materialTextureRes;
		}

		// combinedVertices.insert(combinedVertices.end(), vertices.begin(), vertices.end());
		// combinedIndices.insert(combinedIndices.end(), indices.begin(), indices.end());

		resultMeshDatas.push_back(meshData);
	}

	C::Vector4f averagePos = { 0.f, 0.f, 0.f, 1.f };

	int totalVertices = loaderModel.GetVertices().size();

	//// did this to avoid overflow when adding all vertices below together
	//const float lol = 1.f / totalVertices;

	//for (const auto& vertex : loaderModel.GetVertices())
	//{
	//	averagePos.x += vertex.myPosition.x * lol;
	//	averagePos.y += vertex.myPosition.y * lol;
	//	averagePos.z += vertex.myPosition.z * lol;
	//}

	float maxDistSq = 0.f;

	for (const auto& vertex : loaderModel.GetVertices())
	{
		const auto delta = averagePos - vertex.myPosition;

		const float distSq = delta.LengthSqr();

		if (distSq > maxDistSq)
		{
			maxDistSq = distSq;
		}
	}

	assert(maxDistSq != 0.f);

	const float radius = sqrt(maxDistSq);

	auto vertexBuffer = CreateVertexBuffer(loaderModel.GetVertices());
	auto indexBuffer = CreateIndexBuffer(loaderModel.GetIndices());

	auto fbx = MakeOwned<Model>();

	fbx->myLocalBoundingSphere.InitWithCenterAndRadius(Vec3f(averagePos.x, averagePos.y, averagePos.z), radius);

	for (const auto& node : loaderModel.GetNodes())
	{
		FbxNode newNode;
		newNode.myMeshIndices = node.myMeshIndices;
		newNode.myParentNodeIndex = node.myParentIndex;
		newNode.myTransform = node.myTransform;
		newNode.myName = node.myName;
		newNode.myChildrenIndices = node.myChildrenIndices;

		// Remove the namespace part of the meshname for meshes with namespaces
		const auto colonIndex = newNode.myName.find(':');

		if (colonIndex != std::string::npos)
		{
			newNode.myName = newNode.myName.substr(colonIndex + 1);
		}

		fbx->myNodes.push_back(newNode);
	}

	fbx->myVertexBuffer = vertexBuffer;
	fbx->myIndexBuffer = indexBuffer;
	fbx->myMeshDatas = resultMeshDatas;

	// TODO: Change these here, or not?
	// I think so...not with the vertex shader, not related
	fbx->myOffset = 0;
	fbx->myStride = sizeof(DefaultVertex);

	fbx->myPrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	fbx->myBoneNameToIndex = loaderModel.GetBoneNameToIndex();
	fbx->myBoneOffsets = loaderModel.GetBoneOffsets();
	fbx->myBonesCount = loaderModel.GetBonesCount();
	fbx->myGlobalInverseTransform = loaderModel.GetGlobalInverseTransform();

	return fbx;
}

#if 0
Owned<Engine::Model> Engine::FbxLoader::LoadCustomModel(const Path& aModelPath)
{
	assert(false && "unused");

	const auto parentModelPath = aModelPath.GetParentPath();

	Engine::LoaderModelBinary loader;

	if (!loader.Deserialize(aModelPath))
	{
		LOG_ERROR(LogType::Editor) << "Failed to deserialize: " << aModelPath.ToString();
		return nullptr;
	}

	// std::vector<DefaultVertex> combinedVertices;
	// std::vector<unsigned int> combinedIndices;

	std::vector<MeshData> resultMeshDatas;

	const auto& meshes = loader.GetMeshes();

	for (int meshIndex = 0; meshIndex < meshes.size(); ++meshIndex)
	{
		const auto& loaderMesh = meshes[meshIndex];

		MeshData meshData = {};
		{
			meshData.myMeshName = loaderMesh.myMeshName;

			// Remove the namespace part of the meshname for meshes with namespaces
			const auto colonIndex = meshData.myMeshName.find(':');

			if (colonIndex != std::string::npos)
			{
				meshData.myMeshName = meshData.myMeshName.substr(colonIndex + 1);
			}

			meshData.myVertexCount = loaderMesh.myVerticesCount;
			meshData.myIndicesCount = loaderMesh.myIndicesCount;

			meshData.myBaseVertexLocation = loaderMesh.myVerticesStartIndex;
			meshData.myStartIndexLocation = loaderMesh.myIndicesStartIndex;

			Path albedoTexturePath;

			if (loaderMesh.myAlbedoTexturePath[0] == '\0')
			{
				// no texture with the model, use placeholder
				albedoTexturePath = TextureLoader::GetPlaceholderTexturePath();
			}
			else
			{
				albedoTexturePath = parentModelPath.AppendPath(Path(loaderMesh.myAlbedoTexturePath));
			}

			TextureRef albedoTextureRes =
				myResourceManagerHandler.CreateRef<Engine::TextureResource>(
					albedoTexturePath.ToString());

			albedoTextureRes->RequestLoading();

			Path normalTexturePath;

			if (loaderMesh.myNormalTexturePath[0] == '\0')
			{
				// no texture with the model, use placeholder
				normalTexturePath = TextureLoader::GetPlaceholderTexturePath();
			}
			else
			{
				normalTexturePath = parentModelPath.AppendPath(Path(loaderMesh.myNormalTexturePath));
			}

			TextureRef normalTextureRes =
				myResourceManagerHandler.CreateRef<Engine::TextureResource>(
					normalTexturePath.ToString());

			normalTextureRes->RequestLoading();

			Path materialTexturePath;

			if (loaderMesh.myMaterialTexturePath[0] == '\0')
			{
				// no texture with the model, use placeholder
				materialTexturePath = TextureLoader::GetPlaceholderTexturePath();
			}
			else
			{
				materialTexturePath = parentModelPath.AppendPath(Path(loaderMesh.myMaterialTexturePath));
			}

			TextureRef materialTextureRes =
				myResourceManagerHandler.CreateRef<Engine::TextureResource>(
					materialTexturePath.ToString());

			materialTextureRes->RequestLoading();

			meshData.myTexture[0] = albedoTextureRes;
			meshData.myTexture[1] = normalTextureRes;
			meshData.myTexture[2] = materialTextureRes;
		}

		resultMeshDatas.push_back(meshData);
	}

	//CU::Vector4f averagePos = { 0.f, 0.f, 0.f, 1.f };

	//int totalVertices = 0;

	//for (const auto& mesh : meshes)
	//{
	//	totalVertices += mesh.myVerticesCount;
	//}

	//// did this to avoid overflow when adding all vertices below together
	//const float lol = 1.f / totalVertices;

	//for (int i = 0; i < loader.GetVerticesCount(); ++i)
	//{
	//	averagePos.x += loader.GetVertices()[i].myPosition.x * lol;
	//	averagePos.y += loader.GetVertices()[i].myPosition.y * lol;
	//	averagePos.z += loader.GetVertices()[i].myPosition.z * lol;
	//}

	//float maxDistSq = 0.f;

	//for (int i = 0; i < loader.GetVerticesCount(); ++i)
	//{
	//	const auto delta = averagePos - loader.GetVertices()[i].myPosition;

	//	const float distSq = delta.LengthSqr();

	//	if (distSq > maxDistSq)
	//	{
	//		maxDistSq = distSq;
	//	}
	//}

	//assert(maxDistSq != 0.f);

	//const float radius = sqrt(maxDistSq);

	auto vertexBuffer = CreateVertexBufferRaw(loader.GetVertices(), loader.GetVerticesCount());
	auto indexBuffer = CreateIndexBufferRaw(loader.GetIndices(), loader.GetIndicesCount());

	auto fbx = MakeOwned<Model>();

	//fbx->myLocalBoundingSphere.InitWithCenterAndRadius(CU::Vector3f(averagePos.x, averagePos.y, averagePos.z), radius);

	fbx->myLocalBoundingSphere.InitWithCenterAndRadius(
		loader.GetHeader()->myBoundingSphereCenter,
		loader.GetHeader()->myBoundingSphereRadius);

	fbx->myVertexBuffer = vertexBuffer;
	fbx->myIndexBuffer = indexBuffer;
	fbx->myMeshDatas = resultMeshDatas;

	// TODO: Change these here, or not?
	// I think so...not with the vertex shader, not related
	fbx->myOffset = 0;
	fbx->myStride = sizeof(DefaultVertex);

	fbx->myPrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	for (int i = 0; i < loader.GetBoneToNamesCount(); ++i)
	{
		fbx->myBoneNameToIndex[loader.GetBoneToNames()[i].myName] = loader.GetBoneToNames()[i].myIndex;
	}

	fbx->myBoneOffsets.insert(
		fbx->myBoneOffsets.begin(),
		loader.GetBoneOffsets(),
		loader.GetBoneOffsets() + loader.GetBoneOffsetsCount());

	// fbx->myBoneNameToIndex = loader.GetBoneNameToIndex();
	// fbx->myBoneOffsets = loaderModel.GetBoneOffsets();
	fbx->myBonesCount = loader.GetHeader()->myBonesCount;
	fbx->myGlobalInverseTransform = loader.GetHeader()->myGlobalInverseTransform;

	return fbx;
}
#endif

Owned<Engine::Model> Engine::FbxLoader::LoadCustomModel2(
	const Path& aModelPath,
	ModelResource& aFbxResource)
{
	const auto parentModelPath = aModelPath.GetParentPath();

	BinaryReader reader;

	if (!reader.InitWithFile(aModelPath))
	{
		return nullptr;
	}

	reader.UncompressGzip();

	const auto mdl = TheGame::Sample::GetModel(reader.GetStart());

	std::vector<MeshData> resultMeshDatas;

	for (int meshIndex = 0; meshIndex < mdl->meshes()->size(); ++meshIndex)
	{
		const auto& loaderMesh = mdl->meshes()->Get(meshIndex);

		MeshData meshData = {};
		{
			meshData.myMeshName = loaderMesh->meshname()->str();

			// Remove the namespace part of the meshname for meshes with namespaces
			const auto colonIndex = meshData.myMeshName.find(':');

			if (colonIndex != std::string::npos)
			{
				meshData.myMeshName = meshData.myMeshName.substr(colonIndex + 1);
			}

			if (meshData.myMeshName == "pCylinder1")
			{
				int test = 0;
			}

			meshData.myVertexCount = loaderMesh->vertices_count();
			meshData.myIndicesCount = loaderMesh->indices_count();

			meshData.myBaseVertexLocation = loaderMesh->vertices_start_index();
			meshData.myStartIndexLocation = loaderMesh->indices_start_index();

			Path albedoTexturePath;

			if (loaderMesh->albedo_texture_path()->c_str()[0] == '\0')
			{
				// no texture with the model, use placeholder
				albedoTexturePath = GetEngine().GetPaths().GetPathString(PathEnum::PlaceholderAlbedoTexture);
			}
			else
			{
				albedoTexturePath = Path(loaderMesh->albedo_texture_path()->str());
			}

			TextureRef albedoTextureRes =
				myResourceManagerHandler.CreateRef<TextureResource>(
					albedoTexturePath.ToString());
			aFbxResource.AddChild(albedoTextureRes);

			albedoTextureRes->RequestLoading();

			Path normalTexturePath;

			if (loaderMesh->normal_texture_path()->c_str()[0] == '\0')
			{
				// no texture with the model, use placeholder
				normalTexturePath = GetEngine().GetPaths().GetPathString(PathEnum::PlaceholderNormalTexture);
			}
			else
			{
				normalTexturePath = Path(loaderMesh->normal_texture_path()->str());
			}

			TextureRef normalTextureRes =
				myResourceManagerHandler.CreateRef<TextureResource>(
					normalTexturePath.ToString());
			aFbxResource.AddChild(normalTextureRes);

			normalTextureRes->RequestLoading();

			Path materialTexturePath;

			if (loaderMesh->material_texture_path()->c_str()[0] == '\0')
			{
				// no texture with the model, use placeholder
				materialTexturePath = GetEngine().GetPaths().GetPathString(PathEnum::PlaceholderMaterialTexture);
			}
			else
			{
				materialTexturePath = Path(loaderMesh->material_texture_path()->str());
			}

			TextureRef materialTextureRes =
				myResourceManagerHandler.CreateRef<TextureResource>(
					materialTexturePath.ToString());
			aFbxResource.AddChild(materialTextureRes);

			materialTextureRes->RequestLoading();

			meshData.myTexture[0] = albedoTextureRes;
			meshData.myTexture[1] = normalTextureRes;
			meshData.myTexture[2] = materialTextureRes;



			if (loaderMesh->material_path())
			{
				Path materialPath;

				if (loaderMesh->material_path()->c_str()[0] == '\0')
				{
					materialPath = Path(GetEngine().GetGraphicsEngine().GetMaterialFactory().GetDefaultMeshMaterialDeferred()->GetPath());
				}
				else
				{
					materialPath = Path(loaderMesh->material_path()->str());
				}

				MaterialRef matRef =
					myResourceManagerHandler.CreateRef<MaterialResource>(
						materialPath.ToString());
				aFbxResource.AddChild(matRef);
				matRef->RequestLoading();
				meshData.myDefaultMaterial = matRef;
			}

			if (loaderMesh->local_position())
			{
				meshData.myTransform.SetPositionLocal(
					*reinterpret_cast<const Vec3f*>(loaderMesh->local_position()));
			}

			if (loaderMesh->local_rotation())
			{
				meshData.myTransform.SetRotationLocal(
					Quatf(loaderMesh->local_rotation()->w(),
						Vec3f(
							loaderMesh->local_rotation()->x(),
							loaderMesh->local_rotation()->y(),
							loaderMesh->local_rotation()->z())));
			}

			if (loaderMesh->local_scale())
			{
				meshData.myTransform.SetScaleLocal(
					*reinterpret_cast<const Vec3f*>(loaderMesh->local_scale()));
			}
		}

		resultMeshDatas.push_back(meshData);
	}

	//CU::Vector4f averagePos = { 0.f, 0.f, 0.f, 1.f };

	//int totalVertices = 0;

	//for (const auto& mesh : meshes)
	//{
	//	totalVertices += mesh.myVerticesCount;
	//}

	//// did this to avoid overflow when adding all vertices below together
	//const float lol = 1.f / totalVertices;

	//for (int i = 0; i < loader.GetVerticesCount(); ++i)
	//{
	//	averagePos.x += loader.GetVertices()[i].myPosition.x * lol;
	//	averagePos.y += loader.GetVertices()[i].myPosition.y * lol;
	//	averagePos.z += loader.GetVertices()[i].myPosition.z * lol;
	//}

	//float maxDistSq = 0.f;

	//for (int i = 0; i < loader.GetVerticesCount(); ++i)
	//{
	//	const auto delta = averagePos - loader.GetVertices()[i].myPosition;

	//	const float distSq = delta.LengthSqr();

	//	if (distSq > maxDistSq)
	//	{
	//		maxDistSq = distSq;
	//	}
	//}

	//assert(maxDistSq != 0.f);

	//const float radius = sqrt(maxDistSq);

	//auto d = reinterpret_cast<const DefaultVertex*>(mdl->vertices()->data());
	// auto d = mdl->vertices()->Get(0);
	// float x = d->position().x();

	auto vertexBuffer = CreateVertexBufferRaw((DefaultVertex*)mdl->vertices()->data(), mdl->vertex_count());
	auto indexBuffer = CreateIndexBufferRaw(mdl->indices()->data(), mdl->indices_count());

	auto fbx = MakeOwned<Model>();

	//fbx->myLocalBoundingSphere.InitWithCenterAndRadius(CU::Vector3f(averagePos.x, averagePos.y, averagePos.z), radius);

	fbx->myLocalBoundingSphere.InitWithCenterAndRadius(
		*reinterpret_cast<const Vec3f*>(mdl->bounding_sphere_center()),
		mdl->bounding_sphere_radius());

	fbx->myVertexBuffer = vertexBuffer;
	fbx->myIndexBuffer = indexBuffer;
	fbx->myMeshDatas = resultMeshDatas;

	// TODO: Change these here, or not?
	// I think so...not with the vertex shader, not related
	fbx->myOffset = 0;
	fbx->myStride = sizeof(DefaultVertex);

	fbx->myPrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	for (int i = 0; i < mdl->bone_to_name_count(); ++i)
	{
		fbx->myBoneNameToIndex[mdl->bone_to_names()->Get(i)->name()->str()] = mdl->bone_to_names()->Get(i)->index();
	}

	// fbx->myBoneOffsets.insert(
	// 	fbx->myBoneOffsets.begin(),
	// 	loader.GetBoneOffsets(),
	// 	loader.GetBoneOffsets() + loader.GetBoneOffsetsCount());

	for (int i = 0; i < mdl->bone_offsets()->size(); ++i)
	{
		fbx->myBoneOffsets.push_back(
			*reinterpret_cast<const Mat4f*>(mdl->bone_offsets()->Get(i)->m()));
	}

	// fbx->myBoneNameToIndex = loader.GetBoneNameToIndex();
	// fbx->myBoneOffsets = loaderModel.GetBoneOffsets();
	fbx->myBonesCount = mdl->bones_count();
	fbx->myGlobalInverseTransform = *reinterpret_cast<const Mat4f*>(mdl->global_inverse_transform()->m());

	for (int i = 0; i < mdl->nodes()->size(); ++i)
	{
		const auto& node = mdl->nodes()->Get(i);
		const auto& nodeData = node->data();

		FbxNode newNode;

		for (int j = 0; j < node->mesh_indices()->size(); ++j)
		{
			newNode.myMeshIndices.push_back(node->mesh_indices()->Get(j));
		}

		newNode.myParentNodeIndex = nodeData->parent_node_index();

		newNode.myTransform.SetPositionLocal(Vec3f(
			nodeData->position().x(),
			nodeData->position().y(),
			nodeData->position().z()));

		newNode.myTransform.SetRotationLocal(Quatf(
			nodeData->rotation().w(),
			Vec3f(nodeData->rotation().x(),
				nodeData->rotation().y(),
				nodeData->rotation().z())));

		newNode.myTransform.SetScaleLocal(Vec3f(
			nodeData->scale().x(),
			nodeData->scale().y(),
			nodeData->scale().z()));

		newNode.myName = node->name()->str();

		newNode.myChildrenIndices.resize(node->children_indices()->size());

		for (int j = 0; j < node->children_indices()->size(); ++j)
		{
			newNode.myChildrenIndices[j] = node->children_indices()->Get(j);
		}

		// Remove the namespace part of the meshname for meshes with namespaces
		const auto colonIndex = newNode.myName.find(':');

		if (colonIndex != std::string::npos)
		{
			newNode.myName = newNode.myName.substr(colonIndex + 1);
		}

		fbx->myNodes.push_back(newNode);
	}

	return fbx;
}

std::vector<MeshPointsData> Engine::FbxLoader::LoadMeshDatas(const Path& aModelPath)
{
	std::vector<MeshPointsData> meshes;

	const auto parentModelPath = aModelPath.GetParentPath();

	BinaryReader reader;

	if (!reader.InitWithFile(aModelPath))
	{
		return meshes;
	}

	reader.UncompressGzip();

	const auto mdl = TheGame::Sample::GetModel(reader.GetStart());

	std::vector<MeshData> resultMeshDatas;

	for (int meshIndex = 0; meshIndex < mdl->meshes()->size(); ++meshIndex)
	{
		MeshPointsData meshPointData;

		const auto& loaderMesh = mdl->meshes()->Get(meshIndex);

		MeshData meshData = {};
		{
			meshData.myVertexCount = loaderMesh->vertices_count();
			meshData.myIndicesCount = loaderMesh->indices_count();

			meshData.myBaseVertexLocation = loaderMesh->vertices_start_index();
			meshData.myStartIndexLocation = loaderMesh->indices_start_index();

			//if (loaderMesh->local_position())
			//	meshData.myTransform.SetPositionLocal(
			//		*reinterpret_cast<const Vec3f*>(loaderMesh->local_position()));

			//if (loaderMesh->local_rotation())
			//{
			//	meshData.myTransform.SetRotationLocal(
			//		Quatf(loaderMesh->local_rotation()->w(),
			//			Vec3f(
			//				loaderMesh->local_rotation()->x(),
			//				loaderMesh->local_rotation()->y(),
			//				loaderMesh->local_rotation()->z())));
			//}

			//if (loaderMesh->local_scale())
			//{
			//	meshData.myTransform.SetScaleLocal(
			//		*reinterpret_cast<const Vec3f*>(loaderMesh->local_scale()));
			//}
		}

		resultMeshDatas.push_back(meshData);
		meshes.emplace_back(meshPointData);
	}
	const TheGame::Sample::DefaultVertex* vertices = mdl->vertices()->Get(0);
	const int verticesSize = mdl->vertex_count();
	const int indicesSize = mdl->vertex_count();


	for (int i = 0; i < resultMeshDatas.size(); i++)
	{
		auto md = resultMeshDatas[i];
		auto mesh = meshes[i];

		//mesh.indices.resize(md.myIndicesCount);
		//for (int j = md.myStartIndexLocation; j < md.myStartIndexLocation + md.myIndicesCount; j++)
		//{
		//	mesh.indices[j] = ;
		//}

		//	const auto& node = mdl->nodes()->Get(i);

		//	for (int j = 0; j < node->mesh_indices()->size(); ++j)
		//	{
		//		meshes[i].indices.push_back(node->mesh_indices()->Get(j));
		//	}

		mesh.vertices.resize(md.myVertexCount);
		for (int j = md.myBaseVertexLocation; j < md.myBaseVertexLocation + md.myVertexCount; j++)
		{
			Vec3f pos = { vertices[j].position().x(), vertices[j].position().y(), vertices[j].position().z() };
			mesh.vertices[j] = pos;
		}
	}
	return meshes;
}

ComPtr<ID3D11Buffer> Engine::FbxLoader::CreateVertexBuffer(const std::vector<DefaultVertex>& aVertices)
{
	if (aVertices.empty())
	{
		return nullptr;
	}

	D3D11_BUFFER_DESC vertexBufferDesc = {};
	{
		vertexBufferDesc.ByteWidth = static_cast<UINT>(aVertices.size()) * sizeof(DefaultVertex);
		vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		vertexBufferDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;
	}

	D3D11_SUBRESOURCE_DATA subResData = {};
	{
		subResData.pSysMem = aVertices.data();
	}

	ComPtr<ID3D11Buffer> vertexBuffer = nullptr;

	HRESULT result = myDevice->CreateBuffer(&vertexBufferDesc, &subResData, &vertexBuffer);

	if (FAILED(result))
	{
		LOG_ERROR(LogType::Renderer) << "CreateBuffer failed";
		return nullptr;
	}

	return vertexBuffer;
}

ComPtr<ID3D11Buffer> Engine::FbxLoader::CreateIndexBuffer(const std::vector<unsigned int>& aIndices)
{
	if (aIndices.empty())
	{
		return nullptr;
	}

	D3D11_BUFFER_DESC indexBufferDesc = {};
	{
		indexBufferDesc.ByteWidth = static_cast<UINT>(aIndices.size()) * sizeof(aIndices[0]);
		indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		indexBufferDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_INDEX_BUFFER;
	}

	D3D11_SUBRESOURCE_DATA subResData = {};
	{
		subResData.pSysMem = aIndices.data();
	}

	ComPtr<ID3D11Buffer> indexBuffer = nullptr;

	HRESULT result = myDevice->CreateBuffer(&indexBufferDesc, &subResData, &indexBuffer);

	if (FAILED(result))
	{
		LOG_ERROR(LogType::Renderer) << "CreateBuffer failed";
		return nullptr;
	}

	return indexBuffer;
}

ComPtr<ID3D11Buffer> Engine::FbxLoader::CreateVertexBufferRaw(const DefaultVertex* aVertices, const int aVerticesCount)
{
	if (aVerticesCount <= 0)
	{
		return nullptr;
	}

	D3D11_BUFFER_DESC vertexBufferDesc = {};
	{
		vertexBufferDesc.ByteWidth = static_cast<UINT>(aVerticesCount) * sizeof(DefaultVertex);
		vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		vertexBufferDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;
	}

	D3D11_SUBRESOURCE_DATA subResData = {};
	{
		subResData.pSysMem = aVertices;
	}

	ComPtr<ID3D11Buffer> vertexBuffer = nullptr;

	HRESULT result = myDevice->CreateBuffer(&vertexBufferDesc, &subResData, &vertexBuffer);

	if (FAILED(result))
	{
		LOG_ERROR(LogType::Renderer) << "CreateBuffer failed";
		return nullptr;
	}

	return vertexBuffer;
}

ComPtr<ID3D11Buffer> Engine::FbxLoader::CreateIndexBufferRaw(const unsigned int* aIndices, const int aIndicesCount)
{
	if (aIndicesCount <= 0)
	{
		return nullptr;
	}

	D3D11_BUFFER_DESC indexBufferDesc = {};
	{
		indexBufferDesc.ByteWidth = static_cast<UINT>(aIndicesCount) * sizeof(aIndices[0]);
		indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		indexBufferDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_INDEX_BUFFER;
	}

	D3D11_SUBRESOURCE_DATA subResData = {};
	{
		subResData.pSysMem = aIndices;
	}

	ComPtr<ID3D11Buffer> indexBuffer = nullptr;

	HRESULT result = myDevice->CreateBuffer(&indexBufferDesc, &subResData, &indexBuffer);

	if (FAILED(result))
	{
		LOG_ERROR(LogType::Renderer) << "CreateBuffer failed";
		return nullptr;
	}

	return indexBuffer;
}
