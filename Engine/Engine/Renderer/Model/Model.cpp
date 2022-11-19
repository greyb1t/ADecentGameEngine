#include "pch.h"
#include "Model.h"

Engine::Model::Model()
{
}

Engine::Model::~Model()
{
}

ID3D11Buffer* Engine::Model::GetIndexBuffer()
{
	return myIndexBuffer.Get();
}

ID3D11Buffer* Engine::Model::GetVertexBuffer()
{
	return myVertexBuffer.Get();
}

ID3D11Buffer** Engine::Model::GetVertexBufferPtr()
{
	return myVertexBuffer.GetAddressOf();
}

UINT* Engine::Model::GetStride()
{
	return &myStride;
}

UINT* Engine::Model::GetOffset()
{
	return &myOffset;
}

const std::vector<Engine::MeshData>& Engine::Model::GetMeshDatas() const
{
	return myMeshDatas;
}

const C::Sphere<float>& Engine::Model::GetBoundingSphere() const
{
	return myLocalBoundingSphere;
}

std::vector<Engine::MeshData>& Engine::Model::GetMeshDatas()
{
	return myMeshDatas;
}
