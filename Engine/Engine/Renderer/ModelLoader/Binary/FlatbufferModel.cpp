#include "pch.h"
#include "FlatbufferModel.h"

#include "LoaderModelBinary.h"
#include "model_generated.h"
#include "Common/FileIO.h"
#include "Engine/Renderer/VertexTypes.h"
#include "Engine/Renderer/ModelLoader/LoaderModel.h"

void CalculateBoundingSphere(
	const Engine::LoaderModel& aLoaderModel,
	Vec3f& aCenter, float& aRadius)
{
	/*
	int totalVertices = 0;

	for (const auto& mesh : meshes)
	{
		totalVertices += mesh.myVerticesCount;
	}

	// did this to avoid overflow when adding all vertices below together
	const float lol = 1.f / totalVertices;

	for (int i = 0; i < aLoaderModel.GetVertices().size(); ++i)
	{
		averagePos.x += aLoaderModel.GetVertices()[i].myPosition.x * lol;
		averagePos.y += aLoaderModel.GetVertices()[i].myPosition.y * lol;
		averagePos.z += aLoaderModel.GetVertices()[i].myPosition.z * lol;
	}
	*/

	// NOTE(Filip): No longer calculating average pos due to when we rotate
	// a model it rotatates around its origin and not around the bounding spheres origin
	// that causes it to be incorrect after a rotation
	C::Vector4f averagePos = { 0.f, 0.f, 0.f, 1.f };

	float maxDistSq = 0.f;

	for (int i = 0; i < aLoaderModel.GetVertices().size(); ++i)
	{
		const auto delta = averagePos - aLoaderModel.GetVertices()[i].myPosition;

		const float distSq = delta.LengthSqr();

		if (distSq > maxDistSq)
		{
			maxDistSq = distSq;
		}
	}

	assert(maxDistSq != 0.f);

	const float radius = sqrt(maxDistSq);

	aCenter = averagePos.ToVec3();
	aRadius = radius;
}

void FlatbufferModel::ConvertAndSaveFlatbufferModel(
	const Engine::LoaderModel& aLoaderModel,
	const Path& aPath)
{
	auto& lmb = aLoaderModel;

	flatbuffers::FlatBufferBuilder builder(1024);

	auto verticesVector = builder.CreateVectorOfStructs(
		reinterpret_cast<const TheGame::Sample::DefaultVertex*>(&lmb.GetVertices()[0]),
		lmb.GetVertices().size());

	auto indicesVector = builder.CreateVector(lmb.GetIndices());

	std::vector<flatbuffers::Offset<TheGame::Sample::Mesh>> meshes;

	for (int i = 0; i < lmb.GetMeshes().size(); ++i)
	{
		const auto& m = lmb.GetMeshes()[i];

		auto meshname = builder.CreateString(m.GetName());
		auto albedo = builder.CreateString(m.myAlbedoTexturePath);
		auto normal = builder.CreateString(m.myNormalTexturePath);
		auto material = builder.CreateString(m.myMaterialTexturePath);
		auto materialPath = builder.CreateString(m.myMaterialPath);

		TheGame::Sample::MeshBuilder meshBuilder(builder);
		meshBuilder.add_meshname(meshname);
		meshBuilder.add_albedo_texture_path(albedo);
		meshBuilder.add_normal_texture_path(normal);
		meshBuilder.add_material_texture_path(material);
		meshBuilder.add_vertices_start_index(m.GetVerticesStartIndex());
		meshBuilder.add_vertices_count(m.GetVerticesCount());
		meshBuilder.add_indices_start_index(m.GetIndicesStartIndex());
		meshBuilder.add_indices_count(m.GetIndicesCount());
		meshBuilder.add_material_path(materialPath);

		auto worldPos = m.GetWorldPosition();
		meshBuilder.add_local_position(reinterpret_cast<const TheGame::Sample::Vec3f*>(&worldPos));

		auto worldRot = m.GetWorldRotation();

		auto rot = TheGame::Sample::Quatf(
			worldRot.myVector.x,
			worldRot.myVector.y,
			worldRot.myVector.z,
			worldRot.myW);

		meshBuilder.add_local_rotation(&rot);

		auto worldScale = m.GetWorldScale();
		meshBuilder.add_local_scale(reinterpret_cast<const TheGame::Sample::Vec3f*>(&worldScale));

		auto meshResult = meshBuilder.Finish();
		meshes.push_back(meshResult);
	}

	auto meshesVec = builder.CreateVector(meshes);

	flatbuffers::Offset<flatbuffers::Vector<const TheGame::Sample::Mat4f*>> boneOffsetVector;

	if (!lmb.GetBoneOffsets().empty())
	{
		boneOffsetVector = builder.CreateVectorOfStructs(
			reinterpret_cast<const TheGame::Sample::Mat4f*>(&lmb.GetBoneOffsets()[0]),
			lmb.GetBoneOffsets().size());
	}
	else
	{
		boneOffsetVector = builder.CreateVectorOfStructs<const TheGame::Sample::Mat4f>(nullptr, 0);
	}

	std::vector<flatbuffers::Offset<TheGame::Sample::BoneToName>> bonetonames;

	const auto& boneToName = aLoaderModel.GetBoneNameToIndex();

	if (!boneToName.empty())
	{
		// have to do loop because its a fuckin map
		for (const auto& [name, index] : boneToName)
		{
			auto name2 = builder.CreateString(name);

			TheGame::Sample::BoneToNameBuilder bonetonameBuilder(builder);
			bonetonameBuilder.add_name(name2);
			bonetonameBuilder.add_index(index);
			bonetonames.push_back(bonetonameBuilder.Finish());
		}
	}

	auto bonetonamevecc = builder.CreateVector(bonetonames);

	std::vector<flatbuffers::Offset<TheGame::Sample::Node>> nodes;

	for (const auto& node : aLoaderModel.GetNodes())
	{
		const auto name = builder.CreateString(node.myName);
		const auto meshIndices = builder.CreateVector(node.myMeshIndices);
		const auto childrenIndices = builder.CreateVector(node.myChildrenIndices);

		TheGame::Sample::NodeBuilder nodeBuilder(builder);

		nodeBuilder.add_name(name);

		nodeBuilder.add_mesh_indices(meshIndices);

		nodeBuilder.add_children_indices(childrenIndices);

		TheGame::Sample::NodeData nodeData(
			TheGame::Sample::Vec3f(
				node.myTransform.GetPositionLocal().x,
				node.myTransform.GetPositionLocal().y,
				node.myTransform.GetPositionLocal().z),
			TheGame::Sample::Quatf(
				node.myTransform.GetRotationLocal().myVector.x,
				node.myTransform.GetRotationLocal().myVector.y,
				node.myTransform.GetRotationLocal().myVector.z,
				node.myTransform.GetRotationLocal().myW),
			TheGame::Sample::Vec3f(
				node.myTransform.GetScaleLocal().x,
				node.myTransform.GetScaleLocal().y,
				node.myTransform.GetScaleLocal().z),
			node.myParentIndex
		);

		nodeBuilder.add_data(&nodeData);

		nodes.push_back(nodeBuilder.Finish());
	}

	auto nodesVec = builder.CreateVector(nodes);

	Vec3f boundingSphereCenter;
	float boundingSphereRadius;
	CalculateBoundingSphere(lmb, boundingSphereCenter, boundingSphereRadius);

	TheGame::Sample::ModelBuilder modelBuilder(builder);

	modelBuilder.add_global_inverse_transform(
		reinterpret_cast<const TheGame::Sample::Mat4f*>(
			lmb.GetGlobalInverseTransform().GetPointer()));
	modelBuilder.add_bones_count(lmb.GetBonesCount());
	modelBuilder.add_bounding_sphere_center(
		reinterpret_cast<const TheGame::Sample::Vec3f*>(&boundingSphereCenter));
	modelBuilder.add_bounding_sphere_radius(boundingSphereRadius);

	modelBuilder.add_vertex_count(lmb.GetVertices().size());

	modelBuilder.add_vertices(verticesVector);

	modelBuilder.add_indices_count(lmb.GetIndices().size());

	modelBuilder.add_indices(indicesVector);

	modelBuilder.add_mesh_count(lmb.GetMeshes().size());

	modelBuilder.add_meshes(meshesVec);

	modelBuilder.add_bone_offset_count(lmb.GetBoneOffsets().size());

	modelBuilder.add_bone_offsets(boneOffsetVector);

	modelBuilder.add_bone_to_name_count(lmb.GetBoneNameToIndex().size());

	modelBuilder.add_bone_to_names(bonetonamevecc);

	modelBuilder.add_nodes(nodesVec);

	auto newModel = modelBuilder.Finish();

	builder.Finish(newModel);

	uint8_t* buf = builder.GetBufferPointer();
	int size = builder.GetSize();

	auto originalPath = aPath;

	BinaryWriter binaryWriter;
	binaryWriter.InitWithSize(size);
	binaryWriter.WriteRawMemory(buf, size);
	binaryWriter.Compress(6);
	auto finalBuffer = binaryWriter.GetFinalBuffer();

	FileIO::RemoveReadOnly(originalPath);
	FileIO::WriteBinaryFile(originalPath, finalBuffer);
}
