#pragma once

#include "Engine/ResourceManagement/ResourceRef.h"
#include "Engine/Utils/Flags.h"
#include "Engine/Renderer/BindFlags.h"

namespace Engine
{
	class EffectPass;
	class Model;
	class Material;

	template <typename T>
	class ConstantBuffer;

	class Renderer
	{
	public:
		Renderer(ID3D11DeviceContext& aContext);

		void BindMaterial(const uint16_t aMaterialID, Material& aMaterial);
		void BindEffectPass(EffectPass& aEffectPass, const BindEffectFlags aFlags);
		void BindModelMesh(
			Model& aModel,
			const uint16_t aModelID,
			const uint16_t aMeshIndex,
			const BindModelFlags aFlags);

		template <typename T>
		void BindConstantBufferVS(int aSlot, ConstantBuffer<T>& aConstantBuffer);

		template <typename T>
		void BindConstantBufferPS(int aSlot, ConstantBuffer<T>& aConstantBuffer);

		bool IsBound(EffectPass& aEffectPass) const;
		bool IsModelBound(const uint16_t aModelID, const uint16_t aMeshIndex) const;

		void ResetInternalBindState();

	private:
		ID3D11DeviceContext* myContext = nullptr;

		uint16_t myBoundEffectPassID = -1;
		uint16_t myBoundModelID = -1;
		uint16_t myBoundMeshIndex = -1;
		uint16_t myBoundMaterialID = -1;

		bool myNeededEffectPassCustomValuesUpdate = false;

		const static int ourMaxConstantBufferSlots = 16;
		std::array<uint16_t, ourMaxConstantBufferSlots> myBoundConstantBufferVS;
		std::array<uint16_t, ourMaxConstantBufferSlots> myBoundConstantBufferPS;
	};

	template <typename T>
	void Renderer::BindConstantBufferVS(int aSlot, ConstantBuffer<T>& aConstantBuffer)
	{
		assert(aSlot >= 0 && aSlot < ourMaxConstantBufferSlots);

		if (myBoundConstantBufferVS[aSlot] != aConstantBuffer.GetID())
		{
			myBoundConstantBufferVS[aSlot] = aConstantBuffer.GetID();

			myContext->VSSetConstantBuffers(aSlot, 1, aConstantBuffer.GetAddressOf());
		}
	}

	template <typename T>
	void Renderer::BindConstantBufferPS(int aSlot, ConstantBuffer<T>& aConstantBuffer)
	{
		assert(aSlot >= 0 && aSlot < ourMaxConstantBufferSlots);

		if (myBoundConstantBufferPS[aSlot] != aConstantBuffer.GetID())
		{
			myBoundConstantBufferPS[aSlot] = aConstantBuffer.GetID();

			myContext->PSSetConstantBuffers(aSlot, 1, aConstantBuffer.GetAddressOf());
		}
	}
}
