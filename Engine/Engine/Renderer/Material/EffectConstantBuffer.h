#pragma once

#include "EffectVariables.h"
#include "EffectPassDesc.h"
#include "Engine\Renderer\ShaderFlags.h"

namespace Engine
{
	class EffectConstantBuffer
	{
	public:
		EffectConstantBuffer();
		virtual ~EffectConstantBuffer();

		bool Init(
			const EffectConstantBufferDesc& aLoaderConstantBuffer,
			ID3D11Device* aDevice,
			ID3D11ShaderReflection* aReflection);

		nlohmann::json ToJson() const;

		void MapConstantBufferDataToGPU(ID3D11DeviceContext& aContext) const;
		void BindToPipeline(ID3D11DeviceContext* aContext) const;

		// Returns nullptr if not found
		// otherwise return pointer to value in buffer
		uint8_t* TryGetVariable(const std::string& aVariableName);

		const std::unordered_map<std::string, EffectVariable>& GetVariables() const;

		void MakeUsedByShader(const ShaderFlags aShader);
		bool IsUsedByShader(const ShaderFlags aShader) const;

		const EffectConstantBufferDesc& GetLoaderConstantBuffer() const { return myConstantBufferDesc;  }

	private:
		// temporarily used to save data
		// TODO: Remove this indirection
		EffectConstantBufferDesc myConstantBufferDesc;

		std::string myName;

		std::vector<uint8_t> myConstantBufferData;
		ComPtr<ID3D11Buffer> myBuffer = nullptr;
		std::unordered_map<std::string, EffectVariable> myVariables;
		int mySlot = 0;

		ShaderFlags myShadersUsedIn = ShaderFlags_None;
	};
}