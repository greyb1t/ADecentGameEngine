#pragma once

namespace Engine
{
	class Directx11Framework;

	class ShaderRendererData
	{
	public:
		bool Init(Directx11Framework& aFramework);

		void UpdateDataAndBind();

		// FOR TA
		// TODO: Find a better more organized way to do stuff like this?
		struct SharedBufferData
		{
			float myTotalTime = 0.f;
			float myDeltaTime = 0.f;

			Vec2f myRenderResolution;

			float myRandomValue = 0.f;
			Vec3f _myGarbage;
		} mySharedBufferData;

	private:
		Directx11Framework* myFramework = nullptr;

		ID3D11Buffer* mySharedBuffer = nullptr;
	};
}