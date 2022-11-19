#pragma once

#include "..\ResourceManagement\ResourceRef.h"
#include "..\VertexTypes.h"

namespace Engine
{
	class ResourceReferences;
}

namespace Engine
{
	class Directx11Framework;
	class Camera;
	class WindowHandler;
	class ViewFrustum;
	class PixelShader;
	class VertexShader;
	class FrameBufferTempName;

	class DebugRenderer
	{
	private:
		// TODO: A more organized solution is to make a shader class that is 
		// then inherited from to hold input layout, buffers, and more..?
		// DebugLine3DVertexShader constant buffer structs
		// Register b0
		//struct FrameBufferData
		//{
		//	CU::Matrix4f myToCamera;
		//	CU::Matrix4f myToProjection;
		//} myFrameBufferData = { };

		// Register b1
		struct ObjectBufferData
		{
			Mat4f myToWorld;
		} myObjectBufferData = { };

	public:
		// In normalized space 
		// 0,0 top left corner
		// 1,1 bottom right corner
		void DrawLine2D(
			const Vec2f& aFromNormalized,
			const Vec2f& aToNormalized,
			const C::Vector4f& aColor = ourWhiteColor);

		void DrawCircle2D(
			const Vec2f& aPosNormalized,
			const float aRadiusRadians,
			const C::Vector4f& aColor = ourWhiteColor);

		void DrawRectangle2D(
			const Vec2f& aPosNormalized,
			const Vec2f& aHalfExtents,
			const C::Vector4f& aColor = ourWhiteColor);

		void DrawLine3D(
			const Vec3f& aFrom,
			const Vec3f& aTo,
			const C::Vector4f& aColor = ourWhiteColor,
			const bool aDepthTested = true);

		void DrawCube3D(
			const Vec3f& aCenterWorldPos,
			const Vec3f& aRotationRadians,
			const Vec3f& aSizeHalfExtents = ourOneVector,
			const C::Vector4f& aColor = ourWhiteColor,
			const bool aDepthTested = true);

		void DrawSphere3D(
			const Vec3f& aCenterWorldPos,
			const float aRadiusRadians = 1.f,
			const C::Vector4f& aColor = ourWhiteColor,
			const bool aDepthTested = true);

		void DrawCircle3D(
			const Vec3f& aCenterWorldPos,
			const float aRadiusRadians = 1.f,
			const C::Vector4f& aColor = ourWhiteColor,
			const bool aDepthTested = true);

		void DrawFrustum(
			const Vec3f& aPosition,
			const ViewFrustum& aViewFrustum, 
			const C::Vector4f& aColor = ourWhiteColor,
			const bool aDepthTested = true);

		void DrawFrustumOrthographic(
			const Vec3f& aPosition,
			const ViewFrustum& aViewFrustum,
			const C::Vector4f& aColor = ourWhiteColor,
			const bool aDepthTested = true);

		DebugRenderer(ResourceReferences& aResourceReferences,
			FrameBufferTempName& aFrameBuffer,
			Directx11Framework& aFramework,
			WindowHandler& aWindowHandler);

		bool Init();
		void Render(const Camera* aCamera);
		void RenderDepthTested(const Camera* aCamera);

	private:
		// 2D & 3D
		bool CreatePixelShader();
		bool CreateInputLayout();

		bool CreateVertexBuffer();

		// 2D
		bool CreateVertexShader2DLines();

		// 3D
		bool CreateVertexShader3DLines();

		void Render2DLines();
		void Render3DLines(const Camera* aCamera, const std::vector<LineVertex>& aLineVerticesToRender);

		void AddLineToDraw2D(
			const Vec2f& aFromNormalized, 
			const Vec2f& aToNormalized, 
			const C::Vector4f& aColor);

		void AddLineToDraw3D(
			const Vec3f& aFrom, 
			const Vec3f& aTo, 
			const C::Vector4f& aColor,
			const bool aDepthTested);

	private:
		Directx11Framework& myFramework;
		WindowHandler& myWindowHandler;
		ResourceReferences& myResourceReferences;
		FrameBufferTempName& myFrameBuffer;

		inline static const int ourLineVertexBatchSize = 8192;

		// Both 2D & 3D
		PixelShaderRef myPixelShaderResource;
		ID3D11Buffer* myVertexBuffer = nullptr;

		// 2D
		std::vector<LineVertex> my2dLineVerticesToRender;

		VertexShaderRef myVertexShader2DLinesResource;

		// Same for both
		// ID3D11InputLayout* myInputLayout = nullptr;

		// 3D
		std::vector<LineVertex> my3dLineVerticesToRender;
		std::vector<LineVertex> my3dLineVerticesToRenderDepthTested;

		VertexShaderRef myVertexShader3DLinesResource;

		inline const static C::Vector4f ourWhiteColor = C::Vector4f(1.f, 1.f, 1.f, 1.f);
		inline const static Vec3f ourOneVector = Vec3f(1.f, 1.f, 1.f);
	};
}