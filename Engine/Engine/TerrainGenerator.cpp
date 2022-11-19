#include "pch.h"
#include "TerrainGenerator.h"

#include "Engine/Renderer/VertexTypes.h"
#include "Engine/Renderer/ModelLoader/Binary/LoaderModelBinary.h"
#include "Physics/Physics.h"
#include "Renderer/ModelLoader/Binary/model_generated.h"

//physx::PxShape* TerrainGenerator::Generate(const std::string& aPath)
//{
//	//std::ifstream file(aPath, std::ifstream::binary);
//
//	//if (!file.is_open())
//	//{
//	//	return nullptr;
//	//}
//
//	//file.seekg(0, file.end);
//	//const int length = file.tellg();
//	//file.seekg(0, file.beg);
//
//	//std::vector<uint8_t> buffer;
//	//buffer.resize(length);
//
//	//file.read(reinterpret_cast<char*>(buffer.data()), length);
//
//	//if (buffer.size() == 0)
//	//{
//	//	return nullptr;
//	//}
//
//	//auto* heightMap = reinterpret_cast<int*>(buffer.data());
//	//int rows = heightMap[0];
//	//int cols = heightMap[1];
//	////wrows = 128 + 256;
//	////wcols = 128 + 256;
//
//	//// strip first two indices and then send to physics
//	//heightMap = &heightMap[2];
//
//	//auto* hf = DBG_NEW int_fast16_t[rows * cols];
//	//int i = 0;
//	//
//	//for (int y = 256; y < cols; y++)
//	//{
//	//	for (int x = 256; x < rows; x++)
//	//	{
//	//		hf[i] = static_cast<int_fast16_t>((heightMap[2 + (y * rows + x)]));
//	//	}
//	//}
//	//physx::PxShape* shape = Physics::CreateHeightMap(heightMap, rows, cols);
//
//
//	/*
//	Renderer::LoaderModelBinary loader;
//
//	if (!loader.Deserialize(aPath))
//	{
//		LOG_ERROR(LogType::Editor) << "Failed to deserialize: " << aPath.c_str();
//		return nullptr;
//	}
//	*/
//
//	BinaryReader reader;
//
//	if (!reader.InitWithFile(aPath))
//	{
//		return nullptr;
//	}
//
//	reader.UncompressGzip();
//
//	const auto mdl = MyGame::Sample::GetModel(reader.GetStart());
//
//
//	std::vector<unsigned int> indices(mdl->indices_count());
//	const unsigned int* indicesRaw = mdl->indices()->data();
//	for (int i = 0; i < mdl->indices_count(); i++)
//	{
//		indices[i] = indicesRaw[i];
//	}
//
//	std::vector<Vec3f> vertices(mdl->vertex_count());
//	const DefaultVertex* verticesArr = reinterpret_cast<const DefaultVertex*>(mdl->vertices()->data());
//	for (int i = 0; i < mdl->vertex_count(); i++)
//	{
//		vertices[i] = Vec3f(verticesArr[i].myPosition.x, verticesArr[i].myPosition.y, verticesArr[i].myPosition.z);
//	}
//
//	//physx::PxShape* shape = Physics::CreateTriangleMesh(vertices, indices);
//	
//	return shape;
//}
