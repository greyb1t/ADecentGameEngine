#pragma once

#include "Engine/ResourceManagement\ResourceRef.h"

namespace Engine
{
	class SpriteComponent;
	class ResourceManager;
}

namespace Engine
{
	class Camera;
	class Directx11Framework;
	class SpriteInstance;
	class WindowHandler;

	class SpriteRenderer
	{
	public:
		bool Init(
			ResourceManager& aResourceManager, 
			Directx11Framework& aFramework, 
			WindowHandler& aWindowHandler);

		void Render(const Camera* aCamera, const std::vector<SpriteComponent*>& aSpriteInstances);
		void RenderSingle2D(SpriteComponent* aSprite);

	private:
		bool CreateFrameBuffer();
		bool CreateVertexBuffer();

		struct FrameBufferData
		{
			Mat4f myToCamera;
			Mat4f myToProjection;
			Vec4f myCameraPosition;
			Vec2f myResolution;
			float myNear;
			float myFar;
		} myFrameBufferData;

		Directx11Framework* myFramework = nullptr;
		WindowHandler* myWindowHandler = nullptr;
		ID3D11DeviceContext* myContext = nullptr;
		ID3D11Device* myDevice = nullptr;

		ID3D11Buffer* myFrameBuffer = nullptr;
		ComPtr<ID3D11Buffer> myVertexBuffer;

		GeometryShaderRef myGeometryShader;
	};
}