#pragma once

#include "Engine/Renderer/VertexTypes.h"
#include "Engine/Renderer/RendererType.h"
#include "Engine/Renderer/RenderEnums.h"
#include "MaterialType.h"

namespace Engine
{
	struct EffectStencilDesc
	{
		std::string myType;
		UINT myRef = 0;

		void InitFromJson(const nlohmann::json& j)
		{
			myType = j["Type"];
			myRef = j["Ref"];
		}

		nlohmann::json ToJson() const
		{
			nlohmann::json j;

			j["Type"] = myType;
			j["Ref"] = myRef;

			return j;
		}
	};

	struct EffectTextureDesc
	{
		std::string myTexturePath;
		std::string myNameInShader;

		void InitFromJson(const nlohmann::json& j)
		{
			myTexturePath = j["TexturePath"];
			myNameInShader = j["VariableName"];
		}

		nlohmann::json ToJson() const
		{
			nlohmann::json j;

			j["TexturePath"] = myTexturePath;
			j["VariableName"] = myNameInShader;

			return j;
		}
	};

	struct EffectShaderValueDesc
	{
		std::string Name;
		float Value = 0.f;

		void InitFromJson(const nlohmann::json& j)
		{
			Name = j["Name"];
			Value = j["Value"];
		}
	};

	struct EffectConstantBufferDesc
	{
		struct Layout
		{
			struct Variable
			{
				std::string myName;

				Vec4f myDefaultValue;

				void InitFromJson(const nlohmann::json& j)
				{
					myName = j["Name"];

					if (j.contains("DefaultValue"))
					{
						const auto& defaultValueJson = j["DefaultValue"];

						const auto value = Vec4f(
							defaultValueJson["x"],
							defaultValueJson["y"],
							defaultValueJson["z"],
							defaultValueJson["w"]);

						myDefaultValue = value;
					}
				}

				nlohmann::json ToJson() const
				{
					nlohmann::json j;

					j["Name"] = myName;

					auto& defaultValueJson = j["DefaultValue"];

					defaultValueJson["x"] = myDefaultValue.x;
					defaultValueJson["y"] = myDefaultValue.y;
					defaultValueJson["z"] = myDefaultValue.z;
					defaultValueJson["w"] = myDefaultValue.w;

					return j;
				}
			};

			std::vector<Variable> myVariables;

			void InitFromJson(const nlohmann::json& j);

			nlohmann::json ToJson() const;
		};

		std::string myName;

		Layout myLayout;

		void InitFromJson(const nlohmann::json& j)
		{
			myName = j["Name"];

			myLayout.InitFromJson(j["Layout"]);
		}

		nlohmann::json ToJson() const
		{
			nlohmann::json j;

			j["Name"] = myName;
			j["Layout"] = myLayout.ToJson();

			return j;
		}
	};

	struct EffectPassDesc
	{
		std::string myName;

		RendererType myRenderingType = RendererType::Forward;

		std::string myVertexShaderNamePath;
		// temp solution to support both sprites and models that use differente vertex types
		const D3D11_INPUT_ELEMENT_DESC* myInputLayoutDesc = DefaultVertex::ourInputElements;
		int myInputLayoutElementCount = DefaultVertex::ourElementCount;

		std::string myVertexType;

		std::string myPixelShaderNamePath;

		std::string myGeometryShaderNamePath;

		std::vector<EffectTextureDesc> myTextures;
		std::vector<EffectConstantBufferDesc> myConstantBuffers;

		EffectStencilDesc myStencil;

		RasterizerState myRasterizerState = RasterizerState_NotSet;

		BlendState myBlendState = BlendState_NotSet;

		std::vector<std::string> myShaderDefines;

		float myEmissionIntensity = 1.f;

		void InitFromJson(const nlohmann::json& aJson, const MaterialType aMaterialType);

		nlohmann::json ToJson() const;
	};
}