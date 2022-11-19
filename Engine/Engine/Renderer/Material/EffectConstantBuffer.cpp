#include "pch.h"
#include "EffectConstantBuffer.h"
#include "EffectVariables.h"

Engine::EffectConstantBuffer::EffectConstantBuffer()
{

}

Engine::EffectConstantBuffer::~EffectConstantBuffer()
{

}

bool Engine::EffectConstantBuffer::Init(
	const EffectConstantBufferDesc& aConstantBufferDesc,
	ID3D11Device* aDevice,
	ID3D11ShaderReflection* aReflection)
{
	myConstantBufferDesc = aConstantBufferDesc;

	myName = aConstantBufferDesc.myName;

	/*
		how to handle multiple shaders?
		only one instances of a constant buffer needed
		so send in all shader and reflect on them
		but only created constant buffer if not already created?
	*/

	D3D11_SHADER_INPUT_BIND_DESC bindDesc = { };
	HRESULT result = aReflection->GetResourceBindingDescByName(myName.c_str(), &bindDesc);

	if (FAILED(result))
	{
		// WARNING_LOG("Unable to find constant buffer \"%s\", this can be because "
		// 	"you have not used the ConstantBuffer in HLSL and it being optimized away",
		// 	aLoaderConstantBuffer.myName.c_str());

		return false;
	}

	auto cBufferReflection = aReflection->GetConstantBufferByName(myName.c_str());

	D3D11_SHADER_BUFFER_DESC cBufferDesc = { };
	result = cBufferReflection->GetDesc(&cBufferDesc);

	if (FAILED(result))
	{
		// ERROR_LOG("Unable to find the constant buffer %s", aLoaderConstantBuffer.myName.c_str());
		return false;
	}

	for (const auto& loaderVariable : aConstantBufferDesc.myLayout.myVariables)
	{
		const auto variableReflection = cBufferReflection->GetVariableByName(loaderVariable.myName.c_str());
		D3D11_SHADER_VARIABLE_DESC variableDesc = { };

		result = variableReflection->GetDesc(&variableDesc);

		if (FAILED(result))
		{
			LOG_ERROR(LogType::Renderer) << "Unable to find effect variable " << loaderVariable.myName;
			return false;
		}

		myVariables[loaderVariable.myName] = EffectVariable(variableDesc.StartOffset);

		myConstantBufferData.resize(
			myConstantBufferData.size() + variableDesc.Size);

		*reinterpret_cast<C::Vector4f*>(&myConstantBufferData[variableDesc.StartOffset]) = loaderVariable.myDefaultValue;

		// myVariables[loaderVariable.myName]->SetValueFromJson();
	}

	ID3D11Buffer* cBuffer = nullptr;

	D3D11_BUFFER_DESC bufDesc = { };
	{
		bufDesc.Usage = D3D11_USAGE::D3D11_USAGE_DYNAMIC;
		bufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	}

	bufDesc.ByteWidth = cBufferDesc.Size;

	result = aDevice->CreateBuffer(&bufDesc, nullptr, &cBuffer);

	if (FAILED(result))
	{
		LOG_ERROR(LogType::Renderer) << "CreateBuffer failed to create dynamic material cbuffer";
		return false;
	}

	mySlot = bindDesc.BindPoint;
	myBuffer = cBuffer;

	return true;
}

nlohmann::json Engine::EffectConstantBuffer::ToJson() const
{
	nlohmann::json j;

	j["Name"] = myName;

	nlohmann::json layoutJson = nlohmann::json::array();

	for (const auto& variable : myVariables)
	{
		layoutJson.push_back(variable.second.ToJson());
	}

	j["Layout"] = myConstantBufferDesc.myLayout.ToJson();

	return j;
}

void Engine::EffectConstantBuffer::MapConstantBufferDataToGPU(ID3D11DeviceContext& aContext) const
{
	// Write the values to the buffer
	D3D11_MAPPED_SUBRESOURCE bufferData = { };

	HRESULT result = aContext.Map(
		myBuffer.Get(),
		0,
		D3D11_MAP_WRITE_DISCARD,
		0,
		&bufferData);

	assert(SUCCEEDED(result));

	memcpy(
		bufferData.pData,
		myConstantBufferData.data(),
		myConstantBufferData.size());

	aContext.Unmap(myBuffer.Get(), 0);
}

void Engine::EffectConstantBuffer::BindToPipeline(ID3D11DeviceContext* aContext) const
{
	aContext->PSSetConstantBuffers(
		mySlot,
		1,
		myBuffer.GetAddressOf());

	aContext->VSSetConstantBuffers(
		mySlot,
		1,
		myBuffer.GetAddressOf());
}

uint8_t* Engine::EffectConstantBuffer::TryGetVariable(const std::string& aVariableName)
{
	const auto findResult = myVariables.find(aVariableName);

	const bool exists = findResult != myVariables.end();

	if (exists)
	{
		const int indexToVariableInBufferData = findResult->second.GetIndexToDataBuffer();
		return &myConstantBufferData[indexToVariableInBufferData];
	}
	else
	{
		return nullptr;
	}
}

const std::unordered_map<std::string, Engine::EffectVariable>&
Engine::EffectConstantBuffer::GetVariables() const
{
	return myVariables;
}

void Engine::EffectConstantBuffer::MakeUsedByShader(const ShaderFlags aShader)
{
	myShadersUsedIn |= aShader;
}

bool Engine::EffectConstantBuffer::IsUsedByShader(const ShaderFlags aShader) const
{
	return (myShadersUsedIn & aShader) != 0;
}
