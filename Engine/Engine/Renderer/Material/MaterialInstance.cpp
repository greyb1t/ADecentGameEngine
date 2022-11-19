#include "pch.h"
#include "MaterialInstance.h"
#include "Material.h"
#include "EffectConstantBuffer.h"
#include "EffectPass.h"
#include "Engine/Reflection/Reflector.h"
#include "Engine/ResourceManagement/Resources/MaterialResource.h"

bool Engine::MaterialInstance::Init(const MaterialRef& aMeshMaterial)
{
	assert(aMeshMaterial->GetState() == ResourceState::Loaded && "Must be loaded");

	myMeshMaterial = aMeshMaterial;

	if (aMeshMaterial && aMeshMaterial->IsValid())
	{
		SetDefaultValues(*aMeshMaterial->Get());
	}

	return true;
}

void Engine::MaterialInstance::SetFloat4(const std::string& aVariableName, const C::Vector4f& aValue)
{
	myFloats[aVariableName] = aValue;
	myAreValuesModifiedFromDefault = true;
}

const C::Vector4f& Engine::MaterialInstance::GetFloat4(const std::string& aVariableName)
{
	return myFloats[aVariableName];
}

void Engine::MaterialInstance::UpdateMaterialCustomValues()
{
	// NOTE(filip): removed this if statement because its used in old renderers
	// and I dont want to change them to make this new stuff work
	// if (myAreValuesModifiedFromDefault)
	{
		for (const auto& [name, value] : myFloats)
		{
			GetMaterial()->SetFloat4(name, value);
		}
	}
}

void Engine::MaterialInstance::ResetCustomValuesToDefault()
{
	// Called after rendering to reset the material values to default
	// if (myAreValuesModifiedFromDefault)
	{
		for (const auto& pass : myMeshMaterial->Get()->GetPasses())
		{
			for (const auto& cbuffer : pass->GetConstantBuffers())
			{
				for (const auto& loaderVariable : cbuffer.second->GetLoaderConstantBuffer().myLayout.myVariables)
				{
					GetMaterial()->SetFloat4(loaderVariable.myName, loaderVariable.myDefaultValue);
				}
			}
		}

		/*
		for (const auto& [name, value] : myFloats)
		{
			GetMaterial()->SetFloat4(name, value);
		}

		myMeshMaterial;
		*/
		// SetDefaultValues(*myMeshMaterial->Get());
	}
}

void Engine::MaterialInstance::Reflect(Engine::Reflector& aReflector, const int aMeshIndex)
{
	ReflectMaterial(aReflector);

	if (GetMaterial() == nullptr)
	{
		return;
	}

	for (const auto& pass : GetMaterial()->GetPasses())
	{
		std::vector<std::string> variableNames;

		for (const auto& [cBufferName, cBuffer] : pass->GetConstantBuffers())
		{
			for (const auto& [variableName, _] : cBuffer->GetVariables())
			{
				variableNames.push_back(variableName);
			}
		}

		for (const auto& variableName : variableNames)
		{
			CU::Vector4f& value = myFloats[variableName];
			const CU::Vector4f valueCopy = myFloats[variableName];

			if (aReflector.Reflect(value, variableName) & ReflectorResult_Changing)
			{
				if (value != valueCopy)
				{
					myAreValuesModifiedFromDefault = true;
				}
			}
		}
	}

	aReflector.Reflect(myMainColor, "Main Color", ReflectionFlags_IsColor);
	aReflector.Reflect(myAdditiveColor, "Additive Color", ReflectionFlags_IsColor);

	aReflector.SetNextItemSpeed(0.1f);
	aReflector.SetNextItemRange(0.f, 10000.f);
	aReflector.Reflect(myAdditiveIntensity, "Additive Intensity");
}

Engine::Material* Engine::MaterialInstance::GetMaterial()
{
	if (myMeshMaterial)
	{
		return myMeshMaterial->Get();
	}

	return nullptr;
}

const MaterialRef& Engine::MaterialInstance::GetMaterialRef() const
{
	return myMeshMaterial;
}

bool Engine::MaterialInstance::AreValuesModifiedFromDefault() const
{
	return myAreValuesModifiedFromDefault;
}

void Engine::MaterialInstance::SetAlpha(const float anAlpha)
{
	myMainColor.w = anAlpha;
}

float Engine::MaterialInstance::GetAlpha() const
{
	return myMainColor.w;
}

void Engine::MaterialInstance::SetMainColor(const Vec4f& aColor)
{
	myMainColor = aColor;
}

const Vec4f& Engine::MaterialInstance::GetMainColor() const
{
	return myMainColor;
}

void Engine::MaterialInstance::SetAdditiveColor(const Vec3f& aColor)
{
	myAdditiveColor = aColor;
}

const Vec3f& Engine::MaterialInstance::GetAdditiveColor() const
{
	return myAdditiveColor;
}

void Engine::MaterialInstance::SetAdditiveIntensity(const float aIntensity)
{
	myAdditiveIntensity = aIntensity;
}

float Engine::MaterialInstance::GetAdditiveIntensity() const
{
	return myAdditiveIntensity;
}

Vec4f Engine::MaterialInstance::GetFinalAdditiveColor() const
{
	return Vec4f(myAdditiveColor * myAdditiveIntensity, 0.f);
}

bool Engine::MaterialInstance::IsValid() const
{
	if (myMeshMaterial)
	{
		return myMeshMaterial->IsValid();
	}

	return false;
}

void Engine::MaterialInstance::SetDefaultValues(Material& aMaterial)
{
	// Setting the default values from the material to this instance
	// TODO(filip): Find a better way to accomplish this, this is hardcoded as fuck
	for (const auto& loaderEffectPass : aMaterial.GetPasses())
	{
		for (auto& loaderConstantBuffer : loaderEffectPass->GetConstantBuffers())
		{
			for (const auto& loaderVariable : loaderConstantBuffer.second->GetLoaderConstantBuffer().myLayout.myVariables)
			{
				bool exists = false;

				for (auto& pass : aMaterial.GetPasses())
				{
					for (const auto& cbuffer : pass->GetConstantBuffers())
					{
						if (cbuffer.second->TryGetVariable(loaderVariable.myName))
						{
							exists = true;
							break;
						}
					}

					if (exists)
					{
						break;
					}
				}

				if (!exists)
				{
					continue;
				}

				myFloats[loaderVariable.myName] = loaderVariable.myDefaultValue;
			}
		}
	}
}
