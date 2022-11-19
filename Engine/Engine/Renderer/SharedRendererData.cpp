#include "pch.h"
#include "SharedRendererData.h"
#include "Directx11Framework.h"
#include "WindowHandler.h"
#include "Material\Material.h"
#include "Engine\Time.h"

bool Engine::ShaderRendererData::Init(Directx11Framework& aFramework)
{
	myFramework = &aFramework;

	D3D11_BUFFER_DESC bufDesc = {};
	{
		bufDesc.Usage = D3D11_USAGE::D3D11_USAGE_DYNAMIC;
		bufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	}

	bufDesc.ByteWidth = sizeof(SharedBufferData);

	HRESULT result = aFramework.GetDevice()->CreateBuffer(&bufDesc, nullptr, &mySharedBuffer);

	if (FAILED(result))
	{
		LOG_ERROR(LogType::Renderer) << "CreateBuffer failed to create shared cbuffer";
		return false;
	}

	return true;
}

void Engine::ShaderRendererData::UpdateDataAndBind()
{
	auto context = myFramework->GetContext();

	// TODO: Find a more elegant solution for these kind of stuff
	// Shared buffer update
	{
		mySharedBufferData = {};

		mySharedBufferData.myDeltaTime = Time::DeltaTime;
		mySharedBufferData.myTotalTime = Time::TotalTime;
		mySharedBufferData.myRandomValue = 0.f;
		mySharedBufferData.myRenderResolution = myFramework->GetWindowHandler().GetRenderingSize<float>();

		D3D11_MAPPED_SUBRESOURCE bufferData;
		ZeroMemory(&bufferData, sizeof(bufferData));

		// TODO: make a utils function to update a constant buffer, its a mess
		HRESULT result = context->Map(mySharedBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &bufferData);

		assert(SUCCEEDED(result));

		memcpy(bufferData.pData, &mySharedBufferData, sizeof(mySharedBufferData));

		context->Unmap(mySharedBuffer, 0);

		// 10 is the contant buffer register in Shared.hlsli
		// TODO: Remove the magic numbers everywhere and put them in a good location together?
		context->VSSetConstantBuffers(10, 1, &mySharedBuffer);
		context->PSSetConstantBuffers(10, 1, &mySharedBuffer);
	}
}
