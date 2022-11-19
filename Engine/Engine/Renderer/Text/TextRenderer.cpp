#include "pch.h"
#include "TextRenderer.h"
#include "../Directx11Framework.h"
#include "../Camera/Camera.h"
#include "Text.h"
#include "Font.h"
#include "../ResourceManagement/Resources/PixelShaderResource.h"
#include "../ResourceManagement/Resources/VertexShaderResource.h"
#include "Engine/Renderer/Shaders/PixelShader.h"
#include "Engine/Renderer/Shaders/VertexShader.h"
#include "../VertexTypes.h"
#include "../ResourceManagement/ResourceManager.h"
#include "../Utils/DxUtils.h"

Engine::TextRenderer::TextRenderer(Engine::ResourceManager& aResourceManager, Directx11Framework& aFramework)
	: myResourceManager(aResourceManager),
	myFramework(aFramework)
{
}

bool Engine::TextRenderer::Init()
{
	if (!CreateFrameBuffer())
	{
		return false;
	}

	if (!CreateObjectBuffer())
	{
		return false;
	}

	if (!CreatePixelShader())
	{
		return false;
	}

	if (!CreateVertexShader())
	{
		return false;
	}

	if (!CreateInputLayout())
	{
		return false;
	}

	return true;
}

Engine::TextRendererStatistics Engine::TextRenderer::Render(const Camera* aCamera, const std::vector<Text*> aTexts)
{
	ZoneNamedN(zone1, "TextRenderer::Render", true);

	TextRendererStatistics stats;

	START_TIMER(RenderTimer);

	HRESULT result = 0;
	D3D11_MAPPED_SUBRESOURCE bufferData;

	auto context = myFramework.GetContext();

	// Update framebuffer
	{
		myFrameBufferData.myToCamera = Mat4f::GetFastInverse(aCamera->GetTransform().ToMatrix());
		myFrameBufferData.myToProjection = aCamera->GetProjectionMatrix();

		myFrameBufferData.myCameraPosition = C::Vector4f(
			aCamera->GetTransform().GetPosition().x,
			aCamera->GetTransform().GetPosition().y,
			aCamera->GetTransform().GetPosition().z,
			1.f);

		myFrameBufferData.myNearPlane = aCamera->GetNearPlane();
		myFrameBufferData.myFarPlane = aCamera->GetFarPlane();

		myFrameBufferData.myRenderMode = 0;

		bufferData = {};

		result = context->Map(myFrameBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &bufferData);
		ThrowIfFailed(result);

		memcpy(bufferData.pData, &myFrameBufferData, sizeof(myFrameBufferData));

		context->Unmap(myFrameBuffer, 0);
	}

	context->PSSetConstantBuffers(0, 1, &myFrameBuffer);
	context->VSSetConstantBuffers(0, 1, &myFrameBuffer);


	// WTF, not using myPrimitiveTopology in TextData?
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->IASetInputLayout(myVertexShader->Get().GetInputLayout());

	// needed?
	context->GSSetShader(nullptr, nullptr, 0);
	context->VSSetShader(myVertexShader->Get().GetVertexShader(), nullptr, 0);
	context->PSSetShader(myPixelShader->Get().GetPixelShader(), nullptr, 0);

	for (const auto& text : aTexts)
	{
		const Text::TextData& textData = text->GetTextData();

		if (textData.myNumbersOfVertices <= 0)
		{
			continue;
		}

		// Update object buffer
		{
			myObjectBufferData.myToWorld = text->GetTransform().ToMatrix();

			myObjectBufferData.myColor = text->GetColor();

			bufferData = {};

			result = context->Map(myObjectBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &bufferData);
			ThrowIfFailed(result);

			memcpy(bufferData.pData, &myObjectBufferData, sizeof(myObjectBufferData));

			context->Unmap(myObjectBuffer, 0);
		}


		context->IASetVertexBuffers(0, 1, &textData.myVertexBuffer, &textData.myStride, &textData.myOffset);
		context->IASetIndexBuffer(textData.myIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

		context->VSSetConstantBuffers(1, 1, &myObjectBuffer);
		context->PSSetConstantBuffers(1, 1, &myObjectBuffer);
		context->PSSetShaderResources(0, 1, &text->GetFont()->SRV);

		context->DrawIndexed(textData.myNumberOfIndices, 0, 0);

		stats.myDrawCalls++;
	}

	stats.myRenderTime = END_TIMER_GET_RESULT_MS(RenderTimer);

	return stats;
}

Engine::TextRendererStatistics Engine::TextRenderer::Render2D(const std::vector<Text*> someTexts)
{
	ZoneNamedN(zone1, "TextRenderer::Render", true);

	TextRendererStatistics stats;

	START_TIMER(RenderTimer);

	HRESULT result = 0;
	D3D11_MAPPED_SUBRESOURCE bufferData;

	auto context = myFramework.GetContext();

	// WTF, not using myPrimitiveTopology in TextData?
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->IASetInputLayout(myVertexShader->Get().GetInputLayout());

	// needed?
	context->GSSetShader(nullptr, nullptr, 0);
	context->VSSetShader(my2DVertexShader->Get().GetVertexShader(), nullptr, 0);
	context->PSSetShader(myPixelShader->Get().GetPixelShader(), nullptr, 0);

	for (const auto& text : someTexts)
	{
		const Text::TextData& textData = text->GetTextData();

		if (textData.myNumbersOfVertices <= 0)
		{
			continue;
		}

		// Update object buffer
		{
			Mat4f world = text->GetTransform().ToMatrix();
			world(4, 1) = CU::Lerp(-1.f, 1.f, world(4, 1));
			world(4, 2) = CU::Lerp(1.f, -1.f, world(4, 2));

			myObjectBufferData.myToWorld = world;

			myObjectBufferData.myColor = text->GetColor();

			bufferData = {};

			result = context->Map(myObjectBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &bufferData);
			ThrowIfFailed(result);

			memcpy(bufferData.pData, &myObjectBufferData, sizeof(myObjectBufferData));

			context->Unmap(myObjectBuffer, 0);
		}


		context->IASetVertexBuffers(0, 1, &textData.myVertexBuffer, &textData.myStride, &textData.myOffset);
		context->IASetIndexBuffer(textData.myIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

		context->VSSetConstantBuffers(1, 1, &myObjectBuffer);
		context->PSSetConstantBuffers(1, 1, &myObjectBuffer);
		context->PSSetShaderResources(0, 1, &text->GetFont()->SRV);

		context->DrawIndexed(textData.myNumberOfIndices, 0, 0);

		stats.myDrawCalls++;
	}

	stats.myRenderTime = END_TIMER_GET_RESULT_MS(RenderTimer);

	return stats;
}

void Engine::TextRenderer::RenderSingle2D(Text* aText)
{
	HRESULT result = 0;
	D3D11_MAPPED_SUBRESOURCE bufferData;

	auto context = myFramework.GetContext();

	// WTF, not using myPrimitiveTopology in TextData?
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->IASetInputLayout(myVertexShader->Get().GetInputLayout());

	// needed?
	context->GSSetShader(nullptr, nullptr, 0);
	context->VSSetShader(my2DVertexShader->Get().GetVertexShader(), nullptr, 0);
	context->PSSetShader(myPixelShader->Get().GetPixelShader(), nullptr, 0);

	const Text::TextData& textData = aText->GetTextData();

	if (textData.myNumbersOfVertices <= 0)
	{
		return;
	}

	// Update object buffer
	{
		Mat4f world = aText->GetTransform().ToMatrix();
		world(4, 1) = CU::Lerp(-1.f, 1.f, world(4, 1));
		world(4, 2) = CU::Lerp(1.f, -1.f, world(4, 2));

		myObjectBufferData.myToWorld = world;

		myObjectBufferData.myColor = aText->GetColor();

		bufferData = {};

		result = context->Map(myObjectBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &bufferData);
		ThrowIfFailed(result);

		memcpy(bufferData.pData, &myObjectBufferData, sizeof(myObjectBufferData));

		context->Unmap(myObjectBuffer, 0);
	}


	context->IASetVertexBuffers(0, 1, &textData.myVertexBuffer, &textData.myStride, &textData.myOffset);
	context->IASetIndexBuffer(textData.myIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	context->VSSetConstantBuffers(1, 1, &myObjectBuffer);
	context->PSSetConstantBuffers(1, 1, &myObjectBuffer);
	context->PSSetShaderResources(0, 1, &aText->GetFont()->SRV);

	context->DrawIndexed(textData.myNumberOfIndices, 0, 0);
}

bool Engine::TextRenderer::CreateFrameBuffer()
{
	D3D11_BUFFER_DESC bufDesc = {};
	{
		bufDesc.Usage = D3D11_USAGE::D3D11_USAGE_DYNAMIC;
		bufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bufDesc.ByteWidth = sizeof(FrameBufferData);
	}

	HRESULT result = myFramework.GetDevice()->CreateBuffer(&bufDesc, nullptr, &myFrameBuffer);

	if (FAILED(result))
	{
		LOG_ERROR(LogType::Renderer) << "CreateBuffer failed to create frame cbuffer";
		return false;
	}

	return true;
}

bool Engine::TextRenderer::CreateObjectBuffer()
{
	D3D11_BUFFER_DESC bufDesc = {};
	{
		bufDesc.Usage = D3D11_USAGE::D3D11_USAGE_DYNAMIC;
		bufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bufDesc.ByteWidth = sizeof(ObjectBufferData);
	}

	HRESULT result = myFramework.GetDevice()->CreateBuffer(&bufDesc, nullptr, &myObjectBuffer);

	if (FAILED(result))
	{
		LOG_ERROR(LogType::Renderer) << "CreateBuffer failed to create frame cbuffer";
		return false;
	}

	return true;
}

bool Engine::TextRenderer::CreatePixelShader()
{
	myPixelShader = myResourceManager.CreateRef<Engine::PixelShaderResource>(
		"Assets/Shaders/Engine/Text/TextPixelShader");
	myPixelShader->RequestLoading();

	//if (!myPixelShader->IsValid())
	//{
	//	LOG_ERROR(LogType::Renderer) << "Failed to load TextPixelShader";
	//	return false;
	//}

	return true;
}

bool Engine::TextRenderer::CreateVertexShader()
{
	myVertexShader = myResourceManager.
		CreateRef<Engine::VertexShaderResource>(
			"Assets/Shaders/Engine/Text/TextVertexShader",
			TextVertex::ourInputElements,
			TextVertex::ourElementCount);
	myVertexShader->RequestLoading();

	my2DVertexShader = myResourceManager.CreateRef<Engine::VertexShaderResource>("Assets/Shaders/Engine/Text/TextVertexShader2D",
		TextVertex::ourInputElements,
		TextVertex::ourElementCount);
	my2DVertexShader->RequestLoading();

	//if (!myVertexShader->IsValid())
	//{
	//	LOG_ERROR(LogType::Renderer) << "Failed to load TextVertexShader";
	//	return false;
	//}

	return true;
}

bool Engine::TextRenderer::CreateInputLayout()
{
	//D3D11_INPUT_ELEMENT_DESC layoutDesc[] =
	//{
	//	{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },

	//	{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	//	//{ "COLOR", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	//	//{ "COLOR", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	//	//{ "COLOR", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },

	//	{ "UV", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	//	//{ "UV", 1, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	//	//{ "UV", 2, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	//	//{ "UV", 3, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },

	//	//{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	//	//{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	//	//{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	//};

	//HRESULT result = myFramework->GetDevice()->CreateInputLayout(
	//	layoutDesc,
	//	static_cast<UINT>(std::size(layoutDesc)),
	//	myVertexShader->Get().GetBuffer().data(),
	//	myVertexShader->Get().GetBuffer().size(),
	//	&myInputLayout);

	//if (FAILED(result))
	//{
	//	ERROR_LOG("CreateInputLayout failed");
	//	return false;
	//}

	return true;
}
