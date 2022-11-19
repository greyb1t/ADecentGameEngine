#pragma once

#include "Engine/Utils/Flags.h"
#include "Directx11Framework.h"
#include "Utils/DxUtils.h"

struct ID3D11Buffer;

namespace Engine
{
	extern std::atomic_int myConstantBufferIDCounter;

	enum class ConstantBufferFlags : uint32_t
	{
		Dynamic = 0,
		Writeable = 1 << 0,
	};

	CREATE_FLAG_OF_ENUM(ConstantBufferFlags);

	template <typename T>
	class ConstantBuffer
	{
	public:
		ConstantBuffer() = default;
		~ConstantBuffer() = default;

		bool InitDynamicWritable();

		T& Map(ID3D11DeviceContext& aContext);

		void Unmap(ID3D11DeviceContext& aContext);

		void BindPS(ID3D11DeviceContext& aContext, const int aSlot);
		void BindVS(ID3D11DeviceContext& aContext, const int aSlot);

		ID3D11Buffer* Get() { return myBuffer.Get(); }

		ID3D11Buffer** GetAddressOf() { return myBuffer.GetAddressOf(); }

		uint16_t GetID() const { return myID; }

	private:
		void GenerateID();

	private:
		// ID is used to avoid binding the already bound constant buffer
		uint16_t myID = -1;

		ComPtr<ID3D11Buffer> myBuffer;
	};

	template <typename T>
	void ConstantBuffer<T>::BindVS(ID3D11DeviceContext& aContext, const int aSlot)
	{
		aContext.VSSetConstantBuffers(aSlot, 1, myBuffer.GetAddressOf());
	}

	template <typename T>
	void ConstantBuffer<T>::BindPS(ID3D11DeviceContext& aContext, const int aSlot)
	{
		aContext.PSSetConstantBuffers(aSlot, 1, myBuffer.GetAddressOf());
	}

	template <typename T>
	void ConstantBuffer<T>::Unmap(ID3D11DeviceContext& aContext)
	{
		aContext.Unmap(myBuffer.Get(), 0);
	}

	template <typename T>
	T& ConstantBuffer<T>::Map(ID3D11DeviceContext& aContext)
	{
		D3D11_MAPPED_SUBRESOURCE subRes = { };

		HRESULT result = aContext.Map(
			myBuffer.Get(),
			0,
			D3D11_MAP_WRITE_DISCARD,
			0,
			&subRes);

		ThrowIfFailed(result);

		return *reinterpret_cast<T*>(subRes.pData);
	}

	template <typename T>
	bool ConstantBuffer<T>::InitDynamicWritable()
	{
		GenerateID();

		D3D11_BUFFER_DESC bufDesc = { };
		{
			bufDesc.Usage = D3D11_USAGE::D3D11_USAGE_DYNAMIC;
			bufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			bufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			bufDesc.ByteWidth = sizeof(T);
		}

		HRESULT result = Directx11Framework::GetDevice2().CreateBuffer(
			&bufDesc,
			nullptr,
			myBuffer.GetAddressOf());

		if (FAILED(result))
		{
			LOG_ERROR(LogType::Renderer) << "CreateBuffer failed to create object cbuffer";
			return false;
		}

		return true;
	}

	template <typename T>
	void ConstantBuffer<T>::GenerateID()
	{
		myID = myConstantBufferIDCounter++;
	}
}