#include "pch.h"
#include "PhysicsUtils.h"
#include "Engine/Renderer/Model/Model.h"
#include "Engine/Renderer/ModelLoader/Binary/model_generated.h"
#include "Physics/CookingManager.h"

std::vector<MeshPointsData> PhysicsUtils::CreateTriangleMesh(const std::string& aPath)
{

	std::vector<MeshPointsData> meshes;

	BinaryReader reader;

	if (!reader.InitWithFile(aPath))
	{
		assert(false && "BNruh");
	}

	reader.UncompressGzip();

	const auto mdl = TheGame::Sample::GetModel(reader.GetStart());

	std::vector<Engine::MeshData> resultMeshDatas;

	for (int meshIndex = 0; meshIndex < mdl->meshes()->size(); ++meshIndex)
	{
		MeshPointsData meshPointData;

		const auto& loaderMesh = mdl->meshes()->Get(meshIndex);

		Engine::MeshData meshData = { };
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
	const unsigned int* indices = mdl->indices()->data();
	const int verticesSize = mdl->vertex_count();
	const int indicesSize = mdl->indices_count();

	for (int i = 0; i < resultMeshDatas.size(); i++)
	{
		const auto& md = resultMeshDatas[i];
		auto& mesh = meshes[i];

		const auto& node = mdl->nodes()->Get(i);
		const auto& nodeData = node->data();

		mesh.localPosition = Vec3f(
			nodeData->position().x(),
			nodeData->position().y(),
			nodeData->position().z());

		mesh.localRotation = (Quatf(
			nodeData->rotation().w(),
			Vec3f(nodeData->rotation().x(),
				nodeData->rotation().y(),
				nodeData->rotation().z())));

		mesh.indices.resize(md.myIndicesCount);
		int index = 0;
		for (int j = md.myStartIndexLocation; j < md.myStartIndexLocation + md.myIndicesCount; j++)
		{
			mesh.indices[index] = indices[j];
			++index;
		}

		mesh.vertices.resize(md.myVertexCount);
		index = 0;
		for (int j = md.myBaseVertexLocation; j < md.myBaseVertexLocation + md.myVertexCount; j++)
		{
			Vec3f pos = { vertices[j].position().x(), vertices[j].position().y(), vertices[j].position().z() };
			mesh.vertices[index] = pos;
			++index;
		}
	}

	return meshes;
}
