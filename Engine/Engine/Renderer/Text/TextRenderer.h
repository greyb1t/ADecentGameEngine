#pragma once

#include "../Statistics/RenderingStatistics.h"
#include "../ResourceManagement/ResourceRef.h"

namespace Engine
{
	class ResourceManager;
}

namespace Engine
{
	class Camera;
	class Text;
	class Directx11Framework;

	class TextRenderer
	{
	public:
		TextRenderer(
			ResourceManager& aResourceManager,
			Directx11Framework& aFramework);

		bool Init();

		TextRendererStatistics Render(const Camera* aCamera, const std::vector<Text*> aTexts);
		TextRendererStatistics Render2D(const std::vector<Text*> someTexts);
		void RenderSingle2D(Text* aText);

	private:
		bool CreateFrameBuffer();
		bool CreateObjectBuffer();
		bool CreatePixelShader();
		bool CreateVertexShader();
		bool CreateInputLayout();

	private:
		struct FrameBufferData
		{
			Mat4f myToCamera;
			Mat4f myToProjection;
			C::Vector4f myCameraPosition;
			float myNearPlane;
			float myFarPlane;
			int myRenderMode;
			float garbage;
		} myFrameBufferData;

		struct ObjectBufferData
		{
			Mat4f myToWorld;
			C::Vector4f myColor;
		} myObjectBufferData;

		Directx11Framework& myFramework;
		ResourceManager& myResourceManager;

		ID3D11Buffer* myFrameBuffer = nullptr;
		ID3D11Buffer* myObjectBuffer = nullptr;

		VertexShaderRef myVertexShader;
		VertexShaderRef my2DVertexShader;

		PixelShaderRef myPixelShader;
		// ID3D11InputLayout* myInputLayout = nullptr;
	};
}