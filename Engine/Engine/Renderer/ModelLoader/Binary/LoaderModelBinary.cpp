#include "pch.h"
#include "LoaderModelBinary.h"
#include "..\LoaderModel.h"

std::vector<uint8_t> Engine::LoaderModelBinary::Serialize(const LoaderModel& aLoaderModel)
{
	assert(false && "not used anymore");

	auto binaryWriter = BinaryWriter({});
	binaryWriter.InitWithSize(10000000);

	// Fill with placeholder hader
	const int headerPos = binaryWriter.Write<Header>(Header());

	WriteVertices(binaryWriter, aLoaderModel);
	WriteIndices(binaryWriter, aLoaderModel);

	WriteMeshes(binaryWriter, aLoaderModel);

	WriteBoneOffsets(binaryWriter, aLoaderModel);

	WriteBoneToName(binaryWriter, aLoaderModel);

	Header header;
	header.myBonesCount = aLoaderModel.GetBonesCount();
	header.myGlobalInverseTransform = aLoaderModel.GetGlobalInverseTransform();

	CalculateBoundingSphere(aLoaderModel, header);

	binaryWriter.ReplaceAt(headerPos, header);

	binaryWriter.Compress(6);

	return binaryWriter.GetFinalBuffer();
}

bool Engine::LoaderModelBinary::Deserialize(const Path& aPath)
{
	assert(false && "not used anymore");

	START_TIMER(fileTimer);

	myBinaryStream = BinaryReader({});

	if (!myBinaryStream.InitWithFile(aPath))
	{
		return false;
	}

	myBinaryStream.UncompressGzip();

	myHeader = myBinaryStream.ReadAsPtr<Header>();

	if (myHeader->myMagic[0] != 'G' ||
		myHeader->myMagic[1] != '6' ||
		myHeader->myMagic[2] != 'M')
	{
		LOG_ERROR(LogType::Resource) << aPath << " is not a g6 model file";

		return false;
	}

	float fileMs = END_TIMER_GET_RESULT_MS(fileTimer);

	float verticesMs = 0.f;
	float indicesMs = 0.f;
	float meshesMs = 0.f;
	float boneOffsetMs = 0.f;
	float boneNameMs = 0.f;

	{
		START_TIMER(timer);
		ReadVertices();
		verticesMs = END_TIMER_GET_RESULT_MS(timer);
	}
	{
		START_TIMER(timer);
		ReadIndices();
		indicesMs = END_TIMER_GET_RESULT_MS(timer);
	}
	{
		START_TIMER(timer);
		ReadMeshes();
		meshesMs = END_TIMER_GET_RESULT_MS(timer);
	}
	{
		START_TIMER(timer);
		ReadBoneOffsets();
		boneOffsetMs = END_TIMER_GET_RESULT_MS(timer);
	}
	{
		START_TIMER(timer);
		ReadBoneToName();
		boneNameMs = END_TIMER_GET_RESULT_MS(timer);
	}

	return true;
}

const Engine::LoaderModelBinary::Header* Engine::LoaderModelBinary::GetHeader() const
{
	return myHeader;
}

const DefaultVertex* Engine::LoaderModelBinary::GetVertices() const
{
	return myVertices;
}

int Engine::LoaderModelBinary::GetVerticesCount() const
{
	return myVerticesCount;
}

const unsigned int* Engine::LoaderModelBinary::GetIndices() const
{
	return myIndices;
}

int Engine::LoaderModelBinary::GetIndicesCount() const
{
	return myIndicesCount;
}

const std::vector<Engine::LoaderModelBinary::Mesh>& Engine::LoaderModelBinary::GetMeshes() const
{
	return myMeshes;
}

const Mat4f* Engine::LoaderModelBinary::GetBoneOffsets() const
{
	return myBoneOffsets;
}

int Engine::LoaderModelBinary::GetBoneOffsetsCount() const
{
	return myBoneOffsetsCount;
}

const Engine::LoaderModelBinary::BoneToName* Engine::LoaderModelBinary::GetBoneToNames() const
{
	return myBoneToNames;
}

int Engine::LoaderModelBinary::GetBoneToNamesCount() const
{
	return myBoneToNamesCount;
}

void Engine::LoaderModelBinary::WriteMeshes(BinaryWriter& aBinaryStream, const LoaderModel& aLoaderModel)
{
	const auto& loaderMeshes = aLoaderModel.GetMeshes();

	aBinaryStream.Write<uint32_t>(loaderMeshes.size());

	for (const auto& loaderMesh : loaderMeshes)
	{
		static_assert(sizeof(DefaultVertex) == 120 && "why is this changed?");

		loaderMesh.Serialize(aBinaryStream);
	}
}

void Engine::LoaderModelBinary::WriteBoneOffsets(BinaryWriter& aBinaryStream, const LoaderModel& aLoaderModel)
{
	const auto& boneOffsets = aLoaderModel.GetBoneOffsets();

	aBinaryStream.Write<uint32_t>(boneOffsets.size());

	if (!boneOffsets.empty())
	{
		aBinaryStream.WriteRawMemory(
			reinterpret_cast<const uint8_t*>(&boneOffsets[0]),
			boneOffsets.size() * sizeof(Mat4f));
	}
}

void Engine::LoaderModelBinary::WriteBoneToName(BinaryWriter& aBinaryStream, const LoaderModel& aLoaderModel)
{
	const auto& boneToName = aLoaderModel.GetBoneNameToIndex();

	aBinaryStream.Write<uint32_t>(boneToName.size());

	if (!boneToName.empty())
	{
		std::vector<BoneToName> boneToNamesVector;

		// have to do loop because its a fuckin map
		for (const auto& [name, index] : boneToName)
		{
			BoneToName btn;
			btn.myIndex = index;
			strcpy_s(btn.myName, name.c_str());

			boneToNamesVector.push_back(btn);
		}

		aBinaryStream.WriteRawMemory(
			reinterpret_cast<uint8_t*>(&boneToNamesVector[0]),
			boneToNamesVector.size() * sizeof(BoneToName));
	}
}

void Engine::LoaderModelBinary::ReadVertices()
{
	myVerticesCount = myBinaryStream.Read<uint32_t>();
	myVertices = myBinaryStream.ReadRawMemoryAsPtr<DefaultVertex>(myVerticesCount * sizeof(DefaultVertex));
}

void Engine::LoaderModelBinary::ReadIndices()
{
	myIndicesCount = myBinaryStream.Read<uint32_t>();
	myIndices = myBinaryStream.ReadRawMemoryAsPtr<unsigned int>(myIndicesCount * sizeof(unsigned int));
}

void Engine::LoaderModelBinary::ReadMeshes()
{
	const auto meshCount = myBinaryStream.Read<uint32_t>();

	for (int i = 0; i < meshCount; ++i)
	{
		Mesh mesh;
		mesh.Deserialize(myBinaryStream);
		myMeshes.push_back(mesh);
	}
}

void Engine::LoaderModelBinary::ReadBoneOffsets()
{
	myBoneOffsetsCount = myBinaryStream.Read<uint32_t>();

	if (myBoneOffsetsCount > 0)
	{
		myBoneOffsets = myBinaryStream.ReadRawMemoryAsPtr<Mat4f>(
			myBoneOffsetsCount * sizeof(Mat4f));
	}
}

void Engine::LoaderModelBinary::ReadBoneToName()
{
	myBoneToNamesCount = myBinaryStream.Read<uint32_t>();

	if (myBoneToNamesCount > 0)
	{
		myBoneToNames = myBinaryStream.ReadRawMemoryAsPtr<BoneToName>(
			myBoneToNamesCount * sizeof(BoneToName));
	}
}

void Engine::LoaderModelBinary::CalculateBoundingSphere(const LoaderModel& aLoaderModel, Header& aHeader)
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

	aHeader.myBoundingSphereCenter = averagePos.ToVec3();
	aHeader.myBoundingSphereRadius = radius;
}

void Engine::LoaderModelBinary::WriteVertices(BinaryWriter& aBinaryStream, const LoaderModel& aLoaderModel)
{
	aBinaryStream.Write<uint32_t>(aLoaderModel.myVertices.size());
	aBinaryStream.WriteRawMemory(
		reinterpret_cast<const uint8_t*>(&aLoaderModel.myVertices[0]),
		aLoaderModel.myVertices.size() * sizeof(DefaultVertex));

}

void Engine::LoaderModelBinary::WriteIndices(BinaryWriter& aBinaryStream, const LoaderModel& aLoaderModel)
{
	aBinaryStream.Write<uint32_t>(aLoaderModel.myIndices.size());
	aBinaryStream.WriteRawMemory(
		reinterpret_cast<const uint8_t*>(&aLoaderModel.myIndices[0]),
		aLoaderModel.myIndices.size() * sizeof(unsigned int));
}

void Engine::LoaderModelBinary::Mesh::Deserialize(BinaryReader& aBinaryReader)
{
	myMeshName = aBinaryReader.ReadString();
	myAlbedoTexturePath = aBinaryReader.ReadString();
	myNormalTexturePath = aBinaryReader.ReadString();
	myMaterialTexturePath = aBinaryReader.ReadString();

	myVerticesStartIndex = aBinaryReader.Read<uint32_t>();
	myVerticesCount = aBinaryReader.Read<uint32_t>();

	myIndicesStartIndex = aBinaryReader.Read<uint32_t>();
	myIndicesCount = aBinaryReader.Read<uint32_t>();
}
