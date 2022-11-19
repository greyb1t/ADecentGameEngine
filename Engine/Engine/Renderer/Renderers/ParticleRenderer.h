#pragma once

namespace Engine
{
	class Camera;
	class ParticleEmitterInstance;
	class Directx11Framework;
	class FrameBufferTempName;
	class ParticleEmitterComponent;

	class ParticleRenderer
	{
	public:
		ParticleRenderer(Directx11Framework& aFramework, FrameBufferTempName& aFrameBuffer);

		bool Init();

		void Render(
			const Camera* aCamera,
			const std::vector<ParticleEmitterComponent*>& aParticleEmitters);

	private:
		bool CreateObjectBuffer();

	private:
		Directx11Framework& myFramework;
		ID3D11DeviceContext& myContext;
		ID3D11Device& myDevice;
		FrameBufferTempName& myFrameBuffer;

		//struct FrameBufferData
		//{
		//	CU::Matrix4f myToCamera;
		//	CU::Matrix4f myToProjection;
		//} myFrameBufferData = { };

		struct ObjectBuferData
		{
			Mat4f myToWorld;
		} myObjectBufferData = { };

		ID3D11Buffer* myObjectBuffer = nullptr;
	};
}