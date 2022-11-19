#define NOMINMAX

#include <Windows.h>

#include "Common/Types.h"
#include "Game/Game.h"

#pragma comment(lib, "d3d11.lib")
#ifdef _DEBUG
#pragma comment(lib, "DirectXTK-debug.lib")
#pragma comment(lib, "assimp-vc142-mt.lib")
#pragma comment(lib, "imgui_Debug.lib")
#pragma comment(lib, "imgui_node_editor_Debug.lib")
#pragma comment(lib, "ImGuizmo_Debug.lib")
#else
#pragma comment(lib, "assimp-vc142-mt.lib")
#pragma comment(lib, "DirectXTK-release.lib")
#pragma comment(lib, "imgui_Release.lib")
#pragma comment(lib, "imgui_node_editor_Release.lib")
#pragma comment(lib, "ImGuizmo_Release.lib")
#endif

#include "../tracy-0.7.8/Tracy.hpp"

#ifdef TRACY_ENABLE
void* operator new(std::size_t count)
{
	auto ptr = malloc(count);
	TracyAlloc(ptr, count);
	return ptr;
}

void operator delete(void* ptr) noexcept
{
	TracyFree(ptr);
	free(ptr);
}
#endif

#include "Common/Types.h"
#include "Common/Log.h"
#include "Common/Random.h"
#include "Common/TimerTemp.h"
#include "Common/FileWatcher.h"
#include "Common/nlohmann/json.hpp"
#include "Common/FileIO.h"
#include "Common/JsonManager/JsonManager.h"
#include "Common/Path.h"
#include "Common/BinaryWriter.h"
#include "Common/BinaryReader.h"
#include "Common/Math/Math.h"
#include "Common/Math/PerlinNoise.h"
#include "Common/ObjectPoolGrowable.h"
#include "Common/ObjectPool.h"
#include "Common/ObjectPoolUntyped.h"
#include "Common\StringUtilities.h"
#include "Common/VectorOnStack.h"

namespace C = Common;

template <typename T>
using Vec2 = Common::Vector2<T>;
template <typename T>
using Vec3 = Common::Vector3<T>;
template <typename T>
using Vec4 = Common::Vector4<T>;
using Vec2ui = Common::Vector2ui;
using Vec2f = Common::Vector2f;
using Vec3f = Common::Vector3f;
using Vec4f = Common::Vector4f;
using Quatf = Common::Quaternion;
using Mat4f = Common::Matrix4f;
using Mat3f = Common::Matrix3f;

#include <d3d11.h>
#include <assimp/mesh.h>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include "assimp\quaternion.h"
#include "assimp\vector3.h"
#include "assimp\matrix4x4.h"
#include "assimp\anim.h"
#include <PxPhysicsAPI.h>

#include <iostream>
#include <filesystem>
#include <Common/StringUtilities.h>
#include <Engine/Renderer/ModelLoader/LoaderModel.h>
#include <Common/Path.h>
#include <Engine/Renderer/ModelLoader/Binary/FlatbufferModel.h>
#include "Physics/CookingManager.h"
#include "Physics/Physics.h"

static constexpr int chunkSize = 50000;

namespace std
{
	template <>
	struct hash<DefaultVertex>
	{
		size_t operator()(const DefaultVertex& vertex) const
		{
			using std::hash;

			size_t h = 17;

			h = h * 31 + hash<float>()(vertex.myUVs.x);
			h = h * 31 + hash<float>()(vertex.myUVs.y);

			h = h * 31 + hash<float>()(vertex.myPosition.x);
			h = h * 31 + hash<float>()(vertex.myPosition.y);
			h = h * 31 + hash<float>()(vertex.myPosition.z);
			h = h * 31 + hash<float>()(vertex.myPosition.w);

			h = h * 31 + hash<float>()(vertex.myNormal.x);
			h = h * 31 + hash<float>()(vertex.myNormal.y);
			h = h * 31 + hash<float>()(vertex.myNormal.z);
			h = h * 31 + hash<float>()(vertex.myNormal.w);

			h = h * 31 + hash<float>()(vertex.myBinormal.x);
			h = h * 31 + hash<float>()(vertex.myBinormal.y);
			h = h * 31 + hash<float>()(vertex.myBinormal.z);
			h = h * 31 + hash<float>()(vertex.myBinormal.w);

			h = h * 31 + hash<float>()(vertex.myTangent.x);
			h = h * 31 + hash<float>()(vertex.myTangent.y);
			h = h * 31 + hash<float>()(vertex.myTangent.z);
			h = h * 31 + hash<float>()(vertex.myTangent.w);

			h = h * 31 + hash<float>()(vertex.myColor.x);
			h = h * 31 + hash<float>()(vertex.myColor.y);
			h = h * 31 + hash<float>()(vertex.myColor.z);
			h = h * 31 + hash<float>()(vertex.myColor.w);

			h = h * 31 + hash<float>()(vertex.myBones.x);
			h = h * 31 + hash<float>()(vertex.myBones.y);
			h = h * 31 + hash<float>()(vertex.myBones.z);
			h = h * 31 + hash<float>()(vertex.myBones.w);

			h = h * 31 + hash<float>()(vertex.myWeights.x);
			h = h * 31 + hash<float>()(vertex.myWeights.y);
			h = h * 31 + hash<float>()(vertex.myWeights.z);
			h = h * 31 + hash<float>()(vertex.myWeights.w);

			return h;
		}
	};
}

struct Triangle
{
	int firstIndex, secondIndex, thirdIndex;
};

struct ChunkData
{
	Vec3f myOffset;
	std::string myPath;
};

int GetVertIndex(const std::vector<DefaultVertex>& someVertices, const DefaultVertex& aVertex)
{
	int index = 0;
	for (auto& it : someVertices)
	{
		if (it == aVertex)
		{
			return index;
		}
		++index;
	}
	return -1;
}

ChunkData RemapAndExportToModel(const std::vector<DefaultVertex>& allVertices, const std::vector<Triangle>& someTriangles, const std::string& aFilePath)
{
	std::unordered_map<DefaultVertex, unsigned int> vertToIndex;
	std::vector<DefaultVertex> vertices;
	std::vector<unsigned int> indices;

	for (auto& it : someTriangles)
	{
		const auto& firstVert = allVertices[it.firstIndex];
		const auto& secondVert = allVertices[it.secondIndex];
		const auto& thirdVert = allVertices[it.thirdIndex];

		if (vertToIndex.find(firstVert) == vertToIndex.end())
		{
			vertices.push_back(allVertices[it.firstIndex]);
			vertToIndex.insert({ allVertices[it.firstIndex], vertices.size() - 1 });
		}

		if (vertToIndex.find(secondVert) == vertToIndex.end())
		{
			vertices.push_back(allVertices[it.secondIndex]);
			vertToIndex.insert({ allVertices[it.secondIndex], vertices.size() - 1 });
		}

		if (vertToIndex.find(thirdVert) == vertToIndex.end())
		{
			vertices.push_back(allVertices[it.thirdIndex]);
			vertToIndex.insert({ allVertices[it.thirdIndex], vertices.size() - 1 });
		}

		indices.push_back(vertToIndex[firstVert]);
		indices.push_back(vertToIndex[secondVert]);
		indices.push_back(vertToIndex[thirdVert]);
	}

	// Calculating offset for all vertices.
	Vec3f offset{ 0, 0, 0 };
	for (auto& vert : vertices)
	{
		offset += vert.myPosition.ToVec3();
	}
	offset /= (float)vertices.size();

	// Applying offset to all vertices.
	for (auto& vert : vertices)
	{
		vert.myPosition -= offset.ToVec4(0.0f);
	}

	Owned<Engine::LoaderModel> model = MakeOwned<Engine::LoaderModel>();
	model->Init(vertices, indices);

	FlatbufferModel::ConvertAndSaveFlatbufferModel(*model.get(), aFilePath);

	std::filesystem::path path = aFilePath;
	path.replace_extension(".px");

	std::vector<Vec3f> pxVertices;
	pxVertices.reserve(vertices.size());
	for (const auto& vert : vertices)
	{
		pxVertices.emplace_back(vert.myPosition.ToVec3());
	}
	CookingManager::Instance().CreateTriangleMeshPath(path.string(), pxVertices, indices);

	ChunkData data;
	
	std::filesystem::path relativePath = std::filesystem::relative(aFilePath, std::getenv("COGDOG"));


	data.myPath = "Assets\\" + relativePath.string();
	data.myOffset = offset;
	return data;
}

int main()
{
	std::string converterPath = "\\Meshes\\Environment\\Landscapes\\";
	const char* dogPath = std::getenv("COGDOG");

	Owned<Physics> physics = MakeOwned<Physics>();
	physics->Init();

	if (!dogPath)
	{
		std::cout << "Environment variable COGDOG not set. Make sure it points to Bin/Assets in the correct project." << std::endl;
		return 0;
	}

	std::string path = dogPath;
	path += converterPath;

	for (auto& entry : std::filesystem::directory_iterator(path))
	{
		if (StringUtilities::ToLower(entry.path().extension().string()) != ".fbx")
			continue;


		std::cout << "Converting: " << entry.path().filename().string() << std::endl;

		std::vector<DefaultVertex> vertices;
		std::map<std::pair<int, int>, std::vector<Triangle>> mapping;

		Owned<Assimp::Importer> importer = MakeOwned<Assimp::Importer>();

		unsigned int flags = aiProcessPreset_TargetRealtime_MaxQuality |
			aiProcess_MakeLeftHanded |
			aiProcess_FlipWindingOrder |
			aiProcess_FlipUVs |
			aiProcess_Triangulate |
			aiProcess_CalcTangentSpace |
			// aiProcess_JoinIdenticalVertices |
			aiProcess_SortByPType;

		// Having this line assert becuse to many bones found
		//flags &= ~aiProcess_JoinIdenticalVertices;

		const aiScene* scene = importer->ReadFile(entry.path().string().c_str(), flags);

		int indiceOffset = 0;
		for (int meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex)
		{
			auto mesh = scene->mMeshes[meshIndex];
			vertices.reserve(vertices.size() + mesh->mNumVertices);

			for (int vertIndex = 0; vertIndex < mesh->mNumVertices; ++vertIndex)
			{
				DefaultVertex dVert;
				auto vert = mesh->mVertices[vertIndex];
				dVert.myPosition = { vert.x, vert.y, vert.z, 1.f };
				if (mesh->HasNormals())
				{
					auto normal = mesh->mNormals[vertIndex];
					dVert.myNormal = { normal.x, normal.y, normal.z, 0.0f };
				}

				if (mesh->HasTangentsAndBitangents())
				{
					auto biNormal = mesh->mBitangents[vertIndex];
					auto tangents = mesh->mTangents[vertIndex];
					dVert.myBinormal = { biNormal.x, biNormal.y, biNormal.z, 0.0f };
					dVert.myTangent = { tangents.x, tangents.y, tangents.z, 0.0f };
				}

				if (mesh->HasTextureCoords(vertIndex))
				{
					auto uv = mesh->mTextureCoords[vertIndex][0];
					dVert.myUVs = { uv.x, uv.y };
				}

				if (mesh->HasVertexColors(0))
				{
					auto color = mesh->mColors[0][vertIndex];
					dVert.myColor = { color.r, color.g, color.b, color.a };
				}

				vertices.emplace_back(dVert);
			}

			for (int faceIndex = 0; faceIndex < mesh->mNumFaces; ++faceIndex)
			{
				auto face = mesh->mFaces[faceIndex];

				if (face.mNumIndices < 3)
				{
					std::cout << "ERROR: incorrect indices. Indices with less than 3 faces????? what is this place";
					continue;
				}

				Triangle tri;
				tri.firstIndex = face.mIndices[0] + indiceOffset;
				tri.secondIndex = face.mIndices[1] + indiceOffset;
				tri.thirdIndex = face.mIndices[2] + indiceOffset;

				Vec3f average = vertices[tri.firstIndex].myPosition.ToVec3() + vertices[tri.secondIndex].myPosition.ToVec3() + vertices[tri.thirdIndex].myPosition.ToVec3() / 3.f;
				int x = (int)(average.x / chunkSize);
				int z = (int)(average.z / chunkSize);
				mapping[{x, z }].emplace_back(tri);
			}
			indiceOffset = vertices.size();
		}

		nlohmann::json json;
		for (auto& mEntry : mapping)
		{
			std::string filePath = dogPath + std::string("/Landscapes/");
			filePath += entry.path().stem().string();
			filePath += "/";
			if (!std::filesystem::exists(filePath))
				std::filesystem::create_directories(filePath);
			filePath += "chunk";
			filePath += std::to_string(mEntry.first.first);
			filePath += "-";
			filePath += std::to_string(mEntry.first.second);
			filePath += ".model";

			auto data = RemapAndExportToModel(vertices, mEntry.second, filePath);
			nlohmann::json mappingData;
			mappingData["path"] = data.myPath;
			mappingData["offset"]["x"] = data.myOffset.x;
			mappingData["offset"]["y"] = data.myOffset.y;
			mappingData["offset"]["z"] = data.myOffset.z;
			json.push_back(mappingData);
		}

		std::string landscapePath = dogPath + std::string("/Landscapes/");
		landscapePath += entry.path().stem().string();
		landscapePath += "/";
		landscapePath += entry.path().stem().string();
		landscapePath += ".landscape";

		if (std::filesystem::exists(landscapePath))
			FileIO::RemoveReadOnly(landscapePath);

		std::ofstream stream(landscapePath);
		stream << json.dump(4);
		stream.close();

		std::cout << "Complete! Path: " << landscapePath << std::endl;
	}

	return 0;
}