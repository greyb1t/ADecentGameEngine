#include "pch.h"
#include "FrameConstantBuffer.h"
#include "Directx11Framework.h"
#include "Camera\Camera.h"
#include "Engine/Renderer/Shaders/ShaderConstants.h"
#include "Engine/DebugManager\DebugMenu.h"
#include "Animation\AssimpUtils.h"
#include "Engine\GameObject\Components\EnvironmentLightComponent.h"
#include "PostProcessing\Fog\Fog.h"

bool Engine::FrameBufferTempName::Init(Directx11Framework& aFramework)
{
	myFramework = &aFramework;

	D3D11_BUFFER_DESC bufDesc = {};
	{
		bufDesc.Usage = D3D11_USAGE::D3D11_USAGE_DYNAMIC;
		bufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bufDesc.ByteWidth = sizeof(BufferData);
	}

	HRESULT result = aFramework.GetDevice()->CreateBuffer(&bufDesc, nullptr, &myBuffer);

	if (FAILED(result))
	{
		LOG_ERROR(LogType::Renderer) << "CreateBuffer failed to create frame cbuffer";
		return false;
	}

	return true;
}

void Engine::FrameBufferTempName::UpdateCamera(const Camera& aCamera)
{
	myBufferData.myToCamera = Mat4f::GetFastInverse(aCamera.GetTransform().ToMatrix());
	myBufferData.myFromCameraToWorld = aCamera.GetTransform().ToMatrix();

	myBufferData.myToProjection = aCamera.GetProjectionMatrix();
	//myBufferData.myFromProjectionToCamera = aCamera.GetProjectionMatrix().Inverse();

	// TODO: use our own inverse when I know it works properly
	aiMatrix4x4 mat = CUMatrixToAiMatrix(Mat4f::Transpose(aCamera.GetProjectionMatrix()));
	mat.Inverse();
	myBufferData.myFromProjectionToCamera = Mat4f::Transpose(AiMatrixToCUMatrix(mat));

	myBufferData.myCameraPosition = C::Vector4f(
		aCamera.GetTransform().GetPosition().x,
		aCamera.GetTransform().GetPosition().y,
		aCamera.GetTransform().GetPosition().z,
		1.f);

	myBufferData.myCameraFarPlane = aCamera.GetFarPlane();
	myBufferData.myCameraNearPlane = aCamera.GetNearPlane();

	MapToGPU();
}

void Engine::FrameBufferTempName::UpdateEnvironmentLight(const EnvironmentLightComponent& aEnvironmentLight)
{
	// Since W might be intensity, dont use it in the normalization
	const auto invertedDirection = aEnvironmentLight.GetDirection() * -1.f;
	const auto direction = Vec3f(invertedDirection.x, invertedDirection.y, invertedDirection.z).GetNormalized();
	myBufferData.myToDirectionalLightDirection =
		C::Vector4f(direction.x, direction.y, direction.z,
			aEnvironmentLight.GetDirection().w);
	myBufferData.myDirectionalLightColorAndIntensity =
		C::Vector4f(
			aEnvironmentLight.GetColor().x,
			aEnvironmentLight.GetColor().y,
			aEnvironmentLight.GetColor().z,
			aEnvironmentLight.GetColorIntensity());

	myBufferData.myCubemapIntensity = aEnvironmentLight.GetCubemapIntensity();
	myBufferData.myCubemapInterpolationValue = aEnvironmentLight.GetCubemapInterpolationValue();

	MapToGPU();
}

void Engine::FrameBufferTempName::UpdateFog(const FogData& aFogData)
{
	myBufferData.myFogColor = aFogData.myColor.ToVec4(1.f);

	myBufferData.myFogStart = aFogData.myStart;
	myBufferData.myFogEnd = aFogData.myEnd;

	myBufferData.myFogStrength = aFogData.myFogStrength;

	myBufferData.myHeightFogOffset = aFogData.myHeightFogOffset;
	myBufferData.myHeightFogStrength = aFogData.myHeightFogStrength;

	MapToGPU();
}

void Engine::FrameBufferTempName::UpdateShadowStrength(const float aShadowStrength)
{
	myBufferData.myShadowStrength = aShadowStrength;
}

void Engine::FrameBufferTempName::UpdatePlayerPos(const Vec3f& aPlayerPos)
{
	myBufferData.myPlayerPosition = aPlayerPos;
}

void Engine::FrameBufferTempName::MapToGPU()
{
	D3D11_MAPPED_SUBRESOURCE bufferData;
	ZeroMemory(&bufferData, sizeof(bufferData));

	auto& context = *myFramework->GetContext();
	HRESULT result = context.Map(myBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &bufferData);

	if (FAILED(result))
	{
		LOG_ERROR(LogType::Renderer) << "Context.Map() failed";
		return;
	}

	memcpy(bufferData.pData, &myBufferData, sizeof(myBufferData));

	context.Unmap(myBuffer, 0);

	// TODO: Considering removing bind here, because might be bad for performance
	Bind();
}

void Engine::FrameBufferTempName::Bind()
{
	auto& context = *myFramework->GetContext();

	context.VSSetConstantBuffers(ShaderConstants::FrameBufferSlot, 1, &myBuffer);
	context.PSSetConstantBuffers(ShaderConstants::FrameBufferSlot, 1, &myBuffer);
	context.GSSetConstantBuffers(ShaderConstants::FrameBufferSlot, 1, &myBuffer);
}

void Engine::FrameBufferTempName::DrawDebugMenu()
{
	//Engine::DebugMenu::AddMenuEntry("Frame Buffer", [this]()
	//{
	//	{
	//		ImGui::Separator();

	//		ImGui::Text("Fog");

	//		CU::Vector3f color = myBufferData.myFogColor.ToVec3();
	//		if (ImGui::ColorEdit3("Color", &color.x))
	//		{
	//			myBufferData.myFogColor.x = color.x;
	//			myBufferData.myFogColor.y = color.y;
	//			myBufferData.myFogColor.z = color.z;
	//		}

	//		float fogStart = myBufferData.myFogStart;
	//		if (ImGui::DragFloat("Fog Start", &fogStart, 10.f))
	//		{
	//			myBufferData.myFogStart = fogStart;
	//		}

	//		float fogEnd = myBufferData.myFogEnd;
	//		if (ImGui::DragFloat("Fog End", &fogEnd, 10.f))
	//		{
	//			myBufferData.myFogEnd = fogEnd;
	//		}

	//		ImGui::Separator();
	//	}
	//});
}