#include "pch.h"
#include "DebugRenderer.h"
#include "../Directx11Framework.h"
#include "../Camera/Camera.h"
#include "../WindowHandler.h"
#include "../Culling/ViewFrustum.h"
#include "../ResourceManagement/Resources/PixelShaderResource.h"
#include "../ResourceManagement/Resources/VertexShaderResource.h"
#include "Engine/Renderer/Shaders/VertexShader.h"
#include "Engine/Renderer/Shaders/PixelShader.h"
#include "../VertexTypes.h"
#include "../ResourceManagement/ResourceManager.h"
#include "../FrameConstantBuffer.h"
#include "Engine/ResourceReferences.h"
#include "Engine/Shortcuts.h"

void Engine::DebugRenderer::DrawLine2D(
	const Vec2f& aFromNormalized,
	const Vec2f& aToNormalized,
	const C::Vector4f& aColor)
{
	AddLineToDraw2D(aFromNormalized, aToNormalized, aColor);
}

void Engine::DebugRenderer::DrawCircle2D(
	const Vec2f& aPosNormalized,
	const float aRadiusRadians,
	const C::Vector4f& aColor)
{
	// TODO: Precompute a unit circle, and only upscale it with radians?

	constexpr int TotalPoints = 30;

	Vec2f firstPos;

	Vec2f prevPos;

	const auto resolution = myWindowHandler.GetWindowSize<float>();
	const float aspectRatio = resolution.x / resolution.y;

	for (int i = 0; i < TotalPoints; ++i)
	{
		const float percentOfCircle = static_cast<float>(i) / static_cast<float>(TotalPoints);

		const float radiansOfCircle = Math::PI * 2.f * percentOfCircle;

		const float x = cos(radiansOfCircle) * aRadiusRadians * (1.f / aspectRatio);
		const float y = sin(radiansOfCircle) * aRadiusRadians;

		if (i == 0)
		{
			firstPos = aPosNormalized + Vec2f(x, y);
			prevPos = aPosNormalized + Vec2f(x, y);
			continue;
		}

		const auto toPos = aPosNormalized + Vec2f(x, y);

		AddLineToDraw2D(prevPos, toPos, aColor);

		prevPos = toPos;
	}

	// Koppla (swedish word, cant english) together the last part with the first part of the circle
	AddLineToDraw2D(prevPos, firstPos, aColor);
}

void Engine::DebugRenderer::DrawRectangle2D(
	const Vec2f& aPosNormalized,
	const Vec2f& aHalfExtents,
	const C::Vector4f& aColor)
{
	const auto& center = aPosNormalized;

	const Vec2f cornerPositions[] =
	{
		// Top left
		Vec2f(-aHalfExtents.x, -aHalfExtents.y),

		// Top right
		Vec2f(aHalfExtents.x, -aHalfExtents.y),

		// Bottom right
		Vec2f(aHalfExtents.x, aHalfExtents.y),

		// Bottom left
		Vec2f(-aHalfExtents.x, aHalfExtents.y),
	};

	const int indices[] =
	{
		0, 1,
		1, 2,
		2, 3,
		3, 0
	};

	for (int i = 0; i < std::size(indices) - 1; ++i)
	{
		DrawLine2D(
			cornerPositions[indices[i]] + center,
			cornerPositions[indices[i + 1]] + center,
			aColor);
	}
}

void Engine::DebugRenderer::DrawLine3D(
	const Vec3f& aFrom,
	const Vec3f& aTo,
	const C::Vector4f& aColor,
	const bool aDepthTested)
{
	AddLineToDraw3D(aFrom, aTo, aColor, aDepthTested);
}

void Engine::DebugRenderer::DrawCube3D(
	const Vec3f& aCenterWorldPos,
	const Vec3f& aRotationRadians,
	const Vec3f& aSizeHalfExtents,
	const C::Vector4f& aColor,
	const bool aDepthTested)
{
	const auto& center = aCenterWorldPos;
	const auto& e = aSizeHalfExtents;

	/*
	const auto aRotationMatrix =
		CU::Matrix4f::CreateRotationAroundX(aRotationRadians.x) *
		CU::Matrix4f::CreateRotationAroundY(aRotationRadians.y) *
		CU::Matrix4f::CreateRotationAroundZ(aRotationRadians.z);
	*/

	const auto rotationQuat = Quatf(aRotationRadians);

	const Vec3f cornerPositions[] =
	{
		Vec3f(-e.x, e.y, e.z) * rotationQuat, // 0
		Vec3f(e.x, e.y, e.z) * rotationQuat, // 1
		Vec3f(e.x, -e.y, e.z) * rotationQuat, // 2
		Vec3f(-e.x, -e.y, e.z) * rotationQuat, // 3
		Vec3f(-e.x, -e.y, -e.z) * rotationQuat, // 4
		Vec3f(e.x, -e.y, -e.z) * rotationQuat, // 5
		Vec3f(e.x, e.y, -e.z) * rotationQuat, // 6
		Vec3f(-e.x, e.y, -e.z) * rotationQuat, // 7
	};

	constexpr int sides = 6;
	constexpr int linesPerFace = 8;

	const int indices[sides][linesPerFace] =
	{
		// NOTE: Indices marked with the as using line list
		// We could save some stuff by using line strip? 
		// But then other primitives would be harder?

		// NOTE: Must be in a order of connections, meaning 0 to 1, must 
		// have a line directly to each other on the cube
		// The reason being the way I do the for loop on the indices below

		// TODO: Can optimize because lines are inside of each other, but I dont care =)

		{ 0, 1, 1, 2, 2, 3, 3, 0 }, // +Z Face
		{ 4, 5, 5, 6, 6, 7, 7, 4 }, // -Z Face
		{ 0, 3, 3, 4, 4, 7, 7, 0 }, // -X Face
		{ 1, 2, 2, 5, 5, 6, 6, 1 }, // +X Face
		{ 0, 1, 1, 6, 6, 7, 7, 0 }, // +Y Face
		{ 2, 3, 3, 4, 4, 5, 5, 2 }, // -Y Face
	};

	// Draw each face side
	for (int i = 0; i < sides - 1; ++i)
	{
		// Draw each face lines
		for (int j = 0; j < linesPerFace - 1; ++j)
		{
			DrawLine3D(
				Vec3f(
					cornerPositions[indices[i][j]].x,
					cornerPositions[indices[i][j]].y,
					cornerPositions[indices[i][j]].z) + center,
				Vec3f(
					cornerPositions[indices[i][j + 1]].x,
					cornerPositions[indices[i][j + 1]].y,
					cornerPositions[indices[i][j + 1]].z) + center,
				aColor,
				aDepthTested);
		}
	}
}

void Engine::DebugRenderer::DrawSphere3D(
	const Vec3f& aCenterWorldPos,
	const float aRadiusRadians,
	const C::Vector4f& aColor,
	const bool aDepthTested)
{
	enum Axis
	{
		X = 0,
		Y,
		Z,

		Count
	};

	// For each axis
	for (int axis = X; axis < Axis::Count; axis++)
	{
		constexpr int TotalPoints = 30;

		Vec3f firstPos;
		Vec3f prevPos;

		// Draw a circle
		for (int i = 0; i < TotalPoints; ++i)
		{
			const float percentOfCircle = static_cast<float>(i) / static_cast<float>(TotalPoints);

			const float radiansOfCircle = Math::PI * 2.f * percentOfCircle;

			Vec3f posInCircle;

			switch (axis)
			{
				case X:
					posInCircle = Vec3f(
						cos(radiansOfCircle) * aRadiusRadians,
						sin(radiansOfCircle) * aRadiusRadians,
						0.f);
					break;
				case Y:
					posInCircle = Vec3f(
						cos(radiansOfCircle) * aRadiusRadians,
						0.f,
						sin(radiansOfCircle) * aRadiusRadians);
					break;
				case Z:
					posInCircle = Vec3f(
						0.f,
						cos(radiansOfCircle) * aRadiusRadians,
						sin(radiansOfCircle) * aRadiusRadians);
					break;
				default:
					assert(false && "whot");
					break;
			}

			if (i == 0)
			{
				firstPos = aCenterWorldPos + posInCircle;
				prevPos = aCenterWorldPos + posInCircle;
				continue;
			}

			const auto toPos = aCenterWorldPos + posInCircle;

			AddLineToDraw3D(prevPos, toPos, aColor, aDepthTested);

			prevPos = toPos;
		}

		// Koppla (swedish word, cant english) together the last part with the first part of the circle
		AddLineToDraw3D(prevPos, firstPos, aColor, aDepthTested);
	}
}

void Engine::DebugRenderer::DrawCircle3D(
	const Vec3f& aCenterWorldPos,
	const float aRadiusRadians,
	const C::Vector4f& aColor,
	const bool aDepthTested)
{
	constexpr int TotalPoints = 30;

	Vec3f firstPos;
	Vec3f prevPos;

	// Draw a circle
	for (int i = 0; i < TotalPoints; ++i)
	{
		const float percentOfCircle = static_cast<float>(i) / static_cast<float>(TotalPoints);

		const float radiansOfCircle = Math::PI * 2.f * percentOfCircle;

		Vec3f posInCircle;

		//switch (axis)
		//{
		//	case X:
		//		posInCircle = Vec3f(
		//			cos(radiansOfCircle) * aRadiusRadians,
		//			sin(radiansOfCircle) * aRadiusRadians,
		//			0.f);
		//		break;
		//	case Y:
		//		posInCircle = Vec3f(
		//			cos(radiansOfCircle) * aRadiusRadians,
		//			0.f,
		//			sin(radiansOfCircle) * aRadiusRadians);
		//		break;
		//	case Z:
		//		posInCircle = Vec3f(
		//			0.f,
		//			cos(radiansOfCircle) * aRadiusRadians,
		//			sin(radiansOfCircle) * aRadiusRadians);
		//		break;
		//	default:
		//		assert(false && "whot");
		//		break;
		//}

		posInCircle = Vec3f(
			cos(radiansOfCircle) * aRadiusRadians,
			0.f,
			sin(radiansOfCircle) * aRadiusRadians);

		if (i == 0)
		{
			firstPos = aCenterWorldPos + posInCircle;
			prevPos = aCenterWorldPos + posInCircle;
			continue;
		}

		const auto toPos = aCenterWorldPos + posInCircle;

		AddLineToDraw3D(prevPos, toPos, aColor, aDepthTested);

		prevPos = toPos;
	}

	// Koppla (swedish word, cant english) together the last part with the first part of the circle
	AddLineToDraw3D(prevPos, firstPos, aColor, aDepthTested);
}

void Engine::DebugRenderer::DrawFrustum(
	const Vec3f& aPosition,
	const ViewFrustum& aViewFrustum,
	const C::Vector4f& aColor,
	const bool aDepthTested)
{
	const float scale = 1.f;

	// not multiply pos by scale?
	auto origin = aPosition * scale;

	// Lines from camera to far corners
	// DrawLine3D(origin, origin + (aViewFrustum.ftl * scale));

	// Draw near plane rectangle
	DrawLine3D(aViewFrustum.ntl, aViewFrustum.ntr, aColor, aDepthTested);
	DrawLine3D(aViewFrustum.ntr, aViewFrustum.nbr, aColor, aDepthTested);
	DrawLine3D(aViewFrustum.nbr, aViewFrustum.nbl, aColor, aDepthTested);
	DrawLine3D(aViewFrustum.nbl, aViewFrustum.ntl, aColor, aDepthTested);

	//// Draw far plane rectangle
	DrawLine3D(aViewFrustum.ftl, aViewFrustum.ftr, aColor, aDepthTested);
	DrawLine3D(aViewFrustum.ftr, aViewFrustum.fbr, aColor, aDepthTested);
	DrawLine3D(aViewFrustum.fbr, aViewFrustum.fbl, aColor, aDepthTested);
	DrawLine3D(aViewFrustum.fbl, aViewFrustum.ftl, aColor, aDepthTested);

	// Draw lines from camera to near to far corners
	DrawLine3D(origin, aViewFrustum.ftr, aColor, aDepthTested);
	DrawLine3D(origin, aViewFrustum.fbr, aColor, aDepthTested);
	DrawLine3D(origin, aViewFrustum.fbl, aColor, aDepthTested);
	DrawLine3D(origin, aViewFrustum.ftl, aColor, aDepthTested);
}

void Engine::DebugRenderer::DrawFrustumOrthographic(
	const Vec3f& aPosition,
	const ViewFrustum& aViewFrustum,
	const C::Vector4f& aColor,
	const bool aDepthTested)
{
	// Draw near plane rectangle
	DrawLine3D(aViewFrustum.ntl, aViewFrustum.ntr, aColor, aDepthTested);
	DrawLine3D(aViewFrustum.ntr, aViewFrustum.nbr, aColor, aDepthTested);
	DrawLine3D(aViewFrustum.nbr, aViewFrustum.nbl, aColor, aDepthTested);
	DrawLine3D(aViewFrustum.nbl, aViewFrustum.ntl, aColor, aDepthTested);

	// Draw far plane rectangle
	DrawLine3D(aViewFrustum.ftl, aViewFrustum.ftr, aColor, aDepthTested);
	DrawLine3D(aViewFrustum.ftr, aViewFrustum.fbr, aColor, aDepthTested);
	DrawLine3D(aViewFrustum.fbr, aViewFrustum.fbl, aColor, aDepthTested);
	DrawLine3D(aViewFrustum.fbl, aViewFrustum.ftl, aColor, aDepthTested);

	// Draw far plane rectangle
	DrawLine3D(aViewFrustum.nbl, aViewFrustum.fbl, aColor, aDepthTested);
	DrawLine3D(aViewFrustum.ntl, aViewFrustum.ftl, aColor, aDepthTested);
	DrawLine3D(aViewFrustum.nbr, aViewFrustum.fbr, aColor, aDepthTested);
	DrawLine3D(aViewFrustum.ntr, aViewFrustum.ftr, aColor, aDepthTested);
}

Engine::DebugRenderer::DebugRenderer(
	Engine::ResourceReferences& aResourceReferences,
	FrameBufferTempName& aFrameBuffer,
	Directx11Framework& aFramework,
	WindowHandler& aWindowHandler)
	: myResourceReferences(aResourceReferences),
	myFrameBuffer(aFrameBuffer),
	myFramework(aFramework),
	myWindowHandler(aWindowHandler)
{
}

bool Engine::DebugRenderer::Init()
{
	if (!CreatePixelShader())
	{
		return false;
	}

	if (!CreateVertexShader2DLines())
	{
		return false;
	}

	if (!CreateVertexBuffer())
	{
		return false;
	}

	if (!CreateInputLayout())
	{
		return false;
	}

	if (!CreateVertexShader3DLines())
	{
		return false;
	}

	return true;
}

void Engine::DebugRenderer::Render(const Camera* aCamera)
{
	auto context = myFramework.GetContext();

	context->OMSetBlendState(nullptr, nullptr, 0xffffffff);

	Render2DLines();
	Render3DLines(aCamera, my3dLineVerticesToRender);

	my3dLineVerticesToRender.clear();
}

void Engine::DebugRenderer::RenderDepthTested(const Camera* aCamera)
{
	Render3DLines(aCamera, my3dLineVerticesToRenderDepthTested);

	my3dLineVerticesToRenderDepthTested.clear();
}

bool Engine::DebugRenderer::CreatePixelShader()
{
	myPixelShaderResource = myResourceReferences.AddPixelShader("Assets/Shaders/Engine/Debug/DebugLinePixelShader");
	myPixelShaderResource->RequestLoading();

	//myPixelShaderResource =
	//	myResourceManager.CreateRef<Engine::PixelShaderResource>(
	//		"Debug/DebugLinePixelShader");
	//myPixelShaderResource->RequestLoading();

	//if (!myPixelShaderResource->IsValid())
	//{
	//	LOG_ERROR(LogType::Renderer) << "Failed to load debug pixel shader";
	//	return false;
	//}

	return true;
}

bool Engine::DebugRenderer::CreateVertexShader2DLines()
{
	myVertexShader2DLinesResource =
		GResourceManager->CreateRef<Engine::VertexShaderResource>(
			"Assets/Shaders/Engine/Debug/DebugLineVertexShader",
			LineVertex::ourInputElements,
			LineVertex::ourElementCount);
	myVertexShader2DLinesResource->RequestLoading();

	myResourceReferences.AddVertexShader(myVertexShader2DLinesResource);

	//if (!myVertexShader2DLinesResource->IsValid())
	//{
	//	LOG_ERROR(LogType::Renderer) << "Failed to load DebugLineVertexShader";
	//	return false;
	//}

	return true;
}

bool Engine::DebugRenderer::CreateVertexBuffer()
{
	D3D11_BUFFER_DESC bufferDesc = { };
	{
		bufferDesc.ByteWidth = sizeof(LineVertex) * ourLineVertexBatchSize;
		bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		bufferDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bufferDesc.MiscFlags = 0;
		bufferDesc.StructureByteStride = 0;
	}

	HRESULT result = myFramework.GetDevice()->CreateBuffer(&bufferDesc, nullptr, &myVertexBuffer);

	if (FAILED(result))
	{
		LOG_ERROR(LogType::Renderer) << "Failed to create debug vertex buffer";
		return false;
	}

	return true;
}

bool Engine::DebugRenderer::CreateInputLayout()
{
	//HRESULT result = myFramework.GetDevice()->CreateInputLayout(
	//	LineVertex::ourInputElements,
	//	LineVertex::ourElementCount,
	//	myVertexShader2DLinesResource->Get().GetBuffer().data(),
	//	myVertexShader2DLinesResource->Get().GetBuffer().size(),
	//	&myInputLayout);

	//if (FAILED(result))
	//{
	//	LOG_ERROR(LogType::Renderer) << "CreateInputLayout failed";
	//	return false;
	//}

	return true;
}

bool Engine::DebugRenderer::CreateVertexShader3DLines()
{
	myVertexShader3DLinesResource =
		GResourceManager->CreateRef<Engine::VertexShaderResource>(
			"Assets/Shaders/Engine/Debug/DebugLine3DVertexShader",
			LineVertex::ourInputElements,
			LineVertex::ourElementCount);
	myVertexShader3DLinesResource->RequestLoading();

	myResourceReferences.AddVertexShader(myVertexShader3DLinesResource);

	//if (!myVertexShader3DLinesResource->IsValid())
	//{
	//	LOG_ERROR(LogType::Renderer) << "Failed to load DebugLine3DVertexShader";
	//	return false;
	//}

	return true;
}

void Engine::DebugRenderer::Render2DLines()
{
	auto context = myFramework.GetContext();

	int iterateFromIndex = 0;

	while (iterateFromIndex < my2dLineVerticesToRender.size())
	{
		D3D11_MAPPED_SUBRESOURCE subData = { };

		HRESULT result = context->Map(myVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &subData);

		if (FAILED(result))
		{
			LOG_ERROR(LogType::Renderer) << "Context.Map() failed";
			return;
		}

		// Ensure we never go above the batch size
		const auto linesToIterateCount = std::min(
			ourLineVertexBatchSize,
			static_cast<int>(my2dLineVerticesToRender.size()) - iterateFromIndex);

		LineVertex* vertices = reinterpret_cast<LineVertex*>(subData.pData);
		memcpy(vertices, &my2dLineVerticesToRender[iterateFromIndex], linesToIterateCount * sizeof(LineVertex));

		iterateFromIndex += linesToIterateCount;

		context->Unmap(myVertexBuffer, 0);

		// Draw the batch
		{
			context->IASetInputLayout(myVertexShader2DLinesResource->Get().GetInputLayout());

			context->VSSetShader(myVertexShader2DLinesResource->Get().GetVertexShader(), nullptr, 0);
			context->PSSetShader(myPixelShaderResource->Get().GetPixelShader(), nullptr, 0);

			context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

			// Set vertices
			{
				UINT strides = sizeof(LineVertex);
				UINT offsets = 0;
				context->IASetVertexBuffers(0, 1, &myVertexBuffer, &strides, &offsets);
			}

			context->Draw(linesToIterateCount, 0);
		}
	}

	my2dLineVerticesToRender.clear();
}

void Engine::DebugRenderer::Render3DLines(const Camera* aCamera, const std::vector<LineVertex>& aLineVerticesToRender)
{
	myFrameBuffer.UpdateCamera(*aCamera);

	auto context = myFramework.GetContext();

	int iterateFromIndex = 0;

	while (iterateFromIndex < aLineVerticesToRender.size())
	{
		D3D11_MAPPED_SUBRESOURCE subData = { };

		HRESULT result = context->Map(myVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &subData);

		if (FAILED(result))
		{
			LOG_ERROR(LogType::Renderer) << "Context.Map() failed";
			return;
		}

		// Ensure we never go above the batch size
		const auto linesToIterateCount = std::min(
			ourLineVertexBatchSize,
			static_cast<int>(aLineVerticesToRender.size()) - iterateFromIndex);

		LineVertex* vertices = reinterpret_cast<LineVertex*>(subData.pData);
		memcpy(vertices, &aLineVerticesToRender[iterateFromIndex], linesToIterateCount * sizeof(LineVertex));

		iterateFromIndex += linesToIterateCount;

		context->Unmap(myVertexBuffer, 0);

		// Draw the batch
		{
			context->IASetInputLayout(myVertexShader3DLinesResource->Get().GetInputLayout());

			context->VSSetShader(myVertexShader3DLinesResource->Get().GetVertexShader(), nullptr, 0);
			context->PSSetShader(myPixelShaderResource->Get().GetPixelShader(), nullptr, 0);

			context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

			// Set vertices
			{
				UINT strides = sizeof(LineVertex);
				UINT offsets = 0;
				context->IASetVertexBuffers(0, 1, &myVertexBuffer, &strides, &offsets);
			}

			context->Draw(linesToIterateCount, 0);
		}
	}

	// aLineVerticesToRender.clear();
}

void Engine::DebugRenderer::AddLineToDraw2D(
	const Vec2f& aFromNormalized,
	const Vec2f& aToNormalized,
	const C::Vector4f& aColor)
{
	LineVertex v1;
	{
		v1.myPosition = C::Vector4f(
			aFromNormalized.x,
			aFromNormalized.y,
			0.f,
			1.f);
		v1.myColor = aColor;
	}

	LineVertex v2;
	{
		v2.myPosition = C::Vector4f(
			aToNormalized.x,
			aToNormalized.y,
			0.f,
			1.f);
		v2.myColor = aColor;
	}

	my2dLineVerticesToRender.push_back(v1);
	my2dLineVerticesToRender.push_back(v2);
}

void Engine::DebugRenderer::AddLineToDraw3D(
	const Vec3f& aFrom,
	const Vec3f& aTo,
	const C::Vector4f& aColor,
	const bool aDepthTested)
{
	LineVertex v1;
	{
		v1.myPosition = C::Vector4f(
			aFrom.x,
			aFrom.y,
			aFrom.z,
			1.f);
		v1.myColor = aColor;
	}

	LineVertex v2;
	{
		v2.myPosition = C::Vector4f(
			aTo.x,
			aTo.y,
			aTo.z,
			1.f);
		v2.myColor = aColor;
	}

	if (aDepthTested)
	{
		my3dLineVerticesToRenderDepthTested.push_back(v1);
		my3dLineVerticesToRenderDepthTested.push_back(v2);
	}
	else
	{
		my3dLineVerticesToRender.push_back(v1);
		my3dLineVerticesToRender.push_back(v2);
	}
}