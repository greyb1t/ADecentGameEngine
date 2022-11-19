#include "pch.h"
#include "TextFactory.h"
#include "Font.h"
#include "../VertexTypes.h"

Engine::TextFactory::TextFactory(ID3D11Device* aDevice)
	: myDevice(aDevice)
{
}

bool Engine::TextFactory::Init()
{
	HRESULT result = 0;

	// cgothic
	{
		const std::string fontFileName = "Assets/Fonts/cgothic";
		const std::string atlasFileName = fontFileName + ".dds";
		const std::string fontDefinition = fontFileName + ".json";

		std::ifstream fontDefStream(fontDefinition);

		using nlohmann::json;

		json fontDef;
		fontDefStream >> fontDef;
		fontDefStream.close();

		Font aFont;

		aFont.Atlas.Size = fontDef["atlas"]["size"];
		aFont.Atlas.Width = fontDef["atlas"]["width"];
		aFont.Atlas.Height = fontDef["atlas"]["height"];
		aFont.Atlas.EmSize = fontDef["metrics"]["emSize"];
		aFont.Atlas.LineHeight = fontDef["metrics"]["lineHeight"];
		aFont.Atlas.Ascender = fontDef["metrics"]["ascender"];
		aFont.Atlas.Descender = fontDef["metrics"]["descender"];

		const size_t glyphCount = fontDef["glyphs"].size();

		for (size_t g = 0; g < glyphCount; g++)
		{
			unsigned int unicode = fontDef["glyphs"][g].value("unicode", 0);
			float advance = fontDef["glyphs"][g].value("advance", -1.f);

			C::Vector4f planeBounds = C::Vector4f(0, 0, 0, 0);
			if (fontDef["glyphs"][g].find("planeBounds") != fontDef["glyphs"][g].end())
			{
				planeBounds.x = fontDef["glyphs"][g]["planeBounds"]["left"];
				planeBounds.y = fontDef["glyphs"][g]["planeBounds"]["bottom"];
				planeBounds.z = fontDef["glyphs"][g]["planeBounds"]["right"];
				planeBounds.w = fontDef["glyphs"][g]["planeBounds"]["top"];
			}

			C::Vector4f uvBounds = C::Vector4f(0, 0, 0, 0);

			if (fontDef["glyphs"][g].find("atlasBounds") != fontDef["glyphs"][g].end())
			{
				const float UVStartX = static_cast<float>(fontDef["glyphs"][g]["atlasBounds"]["left"]) / static_cast<float>(aFont.Atlas.Width);
				const float UVStartY = static_cast<float>(fontDef["glyphs"][g]["atlasBounds"]["top"]) / static_cast<float>(aFont.Atlas.Height);

				const float UVEndX = static_cast<float>(fontDef["glyphs"][g]["atlasBounds"]["right"]) / static_cast<float>(aFont.Atlas.Width);
				const float UVEndY = static_cast<float>(fontDef["glyphs"][g]["atlasBounds"]["bottom"]) / static_cast<float>(aFont.Atlas.Height);

				uvBounds = { UVStartX, UVStartY, UVEndX, UVEndY };
			}

			aFont.Glyphs.insert(
				{
					unicode,
				{
					static_cast<char>(unicode),
					advance,
					planeBounds,
					uvBounds
				}
				}
			);
		}

		ID3D11ShaderResourceView* fontSRV = nullptr;

		std::wstring s = std::wstring(atlasFileName.begin(), atlasFileName.end());

		result = DirectX::CreateDDSTextureFromFile(myDevice, s.c_str(), nullptr, &fontSRV);

		if (FAILED(result))
		{
			LOG_ERROR(LogType::Renderer) << "CreateDDSTextureFromFile failed " << atlasFileName;
			return false;
		}

		aFont.SRV = fontSRV;

		myFonts.insert({ "cgothic", aFont });
	}

	// Kenyan coffee bold
	{
		const std::string fontFileName = "Assets/Fonts/kenyan_coffee_bold";
		const std::string atlasFileName = fontFileName + ".dds";
		const std::string fontDefinition = fontFileName + ".json";

		std::ifstream fontDefStream(fontDefinition);

		using nlohmann::json;

		json fontDef;
		fontDefStream >> fontDef;
		fontDefStream.close();

		Font aFont;

		aFont.Atlas.Size = fontDef["atlas"]["size"];
		aFont.Atlas.Width = fontDef["atlas"]["width"];
		aFont.Atlas.Height = fontDef["atlas"]["height"];
		aFont.Atlas.EmSize = fontDef["metrics"]["emSize"];
		aFont.Atlas.LineHeight = fontDef["metrics"]["lineHeight"];
		aFont.Atlas.Ascender = fontDef["metrics"]["ascender"];
		aFont.Atlas.Descender = fontDef["metrics"]["descender"];

		const size_t glyphCount = fontDef["glyphs"].size();

		for (size_t g = 0; g < glyphCount; g++)
		{
			unsigned int unicode = fontDef["glyphs"][g].value("unicode", 0);
			float advance = fontDef["glyphs"][g].value("advance", -1.f);

			C::Vector4f planeBounds = C::Vector4f(0, 0, 0, 0);
			if (fontDef["glyphs"][g].find("planeBounds") != fontDef["glyphs"][g].end())
			{
				planeBounds.x = fontDef["glyphs"][g]["planeBounds"]["left"];
				planeBounds.y = fontDef["glyphs"][g]["planeBounds"]["bottom"];
				planeBounds.z = fontDef["glyphs"][g]["planeBounds"]["right"];
				planeBounds.w = fontDef["glyphs"][g]["planeBounds"]["top"];
			}

			C::Vector4f uvBounds = C::Vector4f(0, 0, 0, 0);

			if (fontDef["glyphs"][g].find("atlasBounds") != fontDef["glyphs"][g].end())
			{
				const float UVStartX = static_cast<float>(fontDef["glyphs"][g]["atlasBounds"]["left"]) / static_cast<float>(aFont.Atlas.Width);
				const float UVStartY = static_cast<float>(fontDef["glyphs"][g]["atlasBounds"]["top"]) / static_cast<float>(aFont.Atlas.Height);

				const float UVEndX = static_cast<float>(fontDef["glyphs"][g]["atlasBounds"]["right"]) / static_cast<float>(aFont.Atlas.Width);
				const float UVEndY = static_cast<float>(fontDef["glyphs"][g]["atlasBounds"]["bottom"]) / static_cast<float>(aFont.Atlas.Height);

				uvBounds = { UVStartX, UVStartY, UVEndX, UVEndY };
			}

			aFont.Glyphs.insert(
				{
					unicode,
				{
					static_cast<char>(unicode),
					advance,
					planeBounds,
					uvBounds
				}
				}
			);
		}

		ID3D11ShaderResourceView* fontSRV = nullptr;

		std::wstring s = std::wstring(atlasFileName.begin(), atlasFileName.end());

		result = DirectX::CreateDDSTextureFromFile(myDevice, s.c_str(), nullptr, &fontSRV);

		if (FAILED(result))
		{
			LOG_ERROR(LogType::Renderer) << "CreateDDSTextureFromFile failed " << atlasFileName;
			return false;
		}

		aFont.SRV = fontSRV;

		myFonts.insert({ "kenyan_coffee_bold", aFont });
	}

	// Kenyan coffee bold
	{
		const std::string fontFileName = "Assets/Fonts/kenyan_coffee_rg";
		const std::string atlasFileName = fontFileName + ".dds";
		const std::string fontDefinition = fontFileName + ".json";

		std::ifstream fontDefStream(fontDefinition);

		using nlohmann::json;

		json fontDef;
		fontDefStream >> fontDef;
		fontDefStream.close();

		Font aFont;

		aFont.Atlas.Size = fontDef["atlas"]["size"];
		aFont.Atlas.Width = fontDef["atlas"]["width"];
		aFont.Atlas.Height = fontDef["atlas"]["height"];
		aFont.Atlas.EmSize = fontDef["metrics"]["emSize"];
		aFont.Atlas.LineHeight = fontDef["metrics"]["lineHeight"];
		aFont.Atlas.Ascender = fontDef["metrics"]["ascender"];
		aFont.Atlas.Descender = fontDef["metrics"]["descender"];

		const size_t glyphCount = fontDef["glyphs"].size();

		for (size_t g = 0; g < glyphCount; g++)
		{
			unsigned int unicode = fontDef["glyphs"][g].value("unicode", 0);
			float advance = fontDef["glyphs"][g].value("advance", -1.f);

			C::Vector4f planeBounds = C::Vector4f(0, 0, 0, 0);
			if (fontDef["glyphs"][g].find("planeBounds") != fontDef["glyphs"][g].end())
			{
				planeBounds.x = fontDef["glyphs"][g]["planeBounds"]["left"];
				planeBounds.y = fontDef["glyphs"][g]["planeBounds"]["bottom"];
				planeBounds.z = fontDef["glyphs"][g]["planeBounds"]["right"];
				planeBounds.w = fontDef["glyphs"][g]["planeBounds"]["top"];
			}

			C::Vector4f uvBounds = C::Vector4f(0, 0, 0, 0);

			if (fontDef["glyphs"][g].find("atlasBounds") != fontDef["glyphs"][g].end())
			{
				const float UVStartX = static_cast<float>(fontDef["glyphs"][g]["atlasBounds"]["left"]) / static_cast<float>(aFont.Atlas.Width);
				const float UVStartY = static_cast<float>(fontDef["glyphs"][g]["atlasBounds"]["top"]) / static_cast<float>(aFont.Atlas.Height);

				const float UVEndX = static_cast<float>(fontDef["glyphs"][g]["atlasBounds"]["right"]) / static_cast<float>(aFont.Atlas.Width);
				const float UVEndY = static_cast<float>(fontDef["glyphs"][g]["atlasBounds"]["bottom"]) / static_cast<float>(aFont.Atlas.Height);

				uvBounds = { UVStartX, UVStartY, UVEndX, UVEndY };
			}

			aFont.Glyphs.insert(
				{
					unicode,
				{
					static_cast<char>(unicode),
					advance,
					planeBounds,
					uvBounds
				}
				}
			);
		}

		ID3D11ShaderResourceView* fontSRV = nullptr;

		std::wstring s = std::wstring(atlasFileName.begin(), atlasFileName.end());

		result = DirectX::CreateDDSTextureFromFile(myDevice, s.c_str(), nullptr, &fontSRV);

		if (FAILED(result))
		{
			LOG_ERROR(LogType::Renderer) << "CreateDDSTextureFromFile failed " << atlasFileName;
			return false;
		}

		aFont.SRV = fontSRV;

		myFonts.insert({ "kenyan_coffee_rg", aFont });
	}

	return true;
}

Engine::Text Engine::TextFactory::CreateText(const std::string& aText, const float aWorldSize, const bool aIsCentered)
{
	if (aText.empty())
	{
		return {};
	}

	std::string fontName = "kenyan_coffee_rg";
	auto It = myFonts.find(fontName);
	Font font = It->second;

	std::vector<TextVertex> vertices;
	std::vector<unsigned int> indices;
	float X = 0.f;
	float actualWorldSize = font.Atlas.Size * aWorldSize;

	float y = 0.f;

	float maxX = 0.f;

	for (char c : aText)
	{
		const float charAdvance = font[c].Advance * actualWorldSize;

		const unsigned int currentVertexCount = static_cast<unsigned>(vertices.size());

		const C::Vector4f Bounds = font[c].UVBounds;
		C::Vector4f Offsets = font[c].PlaneBounds;

		Offsets.x = font[c].PlaneBounds.x * charAdvance;

		Offsets.z = font[c].PlaneBounds.z * actualWorldSize * 0.65f;

		Offsets.y = font[c].PlaneBounds.y * charAdvance;
		Offsets.w = font[c].PlaneBounds.w * actualWorldSize;

		if (abs(Offsets.w) > 0.005f)
		{
			Offsets.w += font.Atlas.Descender * actualWorldSize;
		}

		if (c == '\n')
		{
			y -= font.Atlas.LineHeight * actualWorldSize;
			X = 0.f;
		}

		// Bottom left
		vertices.push_back(TextVertex(
			Vec3f(X + Offsets.x, Offsets.y + y, 0.f),
			C::Vector4f(1.f, 1.f, 1.f, 1.f),
			Vec2f(Bounds.x, 1.f - Bounds.w)
		));

		// Top left
		vertices.push_back(TextVertex(
			Vec3f(X + Offsets.x, Offsets.w + y, 0.f),
			C::Vector4f(1.f, 1.f, 1.f, 1.f),
			Vec2f(Bounds.x, 1.f - Bounds.y)
		));

		// Bottom right
		vertices.push_back(TextVertex(
			Vec3f(X + Offsets.z, Offsets.y + y, 0.f),
			C::Vector4f(1.f, 1.f, 1.f, 1.f),
			Vec2f(Bounds.z, 1.f - Bounds.w)
		));

		// Top right
		vertices.push_back(TextVertex(
			Vec3f(X + Offsets.z, Offsets.w + y, 0.f),
			C::Vector4f(1.f, 1.f, 1.f, 1.f),
			Vec2f(Bounds.z, 1.f - Bounds.y)
		));

		X += charAdvance * 0.65f;

		if(X > maxX)
		{
			maxX = X;
		}

		indices.push_back(currentVertexCount);
		indices.push_back(currentVertexCount + 1);
		indices.push_back(currentVertexCount + 2);
		indices.push_back(currentVertexCount + 1);
		indices.push_back(currentVertexCount + 3);
		indices.push_back(currentVertexCount + 2);
	}

	if(aIsCentered)
	{
		float halfMaxX = maxX * 0.5f;
		float halfY = y * 0.5f;

		for(TextVertex& vertex : vertices)
		{
			vertex.myPosition.x -= halfMaxX;
			vertex.myPosition.y -= halfY;
		}
	}

	ID3D11Buffer* vertexBuffer = nullptr;
	{
		D3D11_BUFFER_DESC vertexBufferDesc = {};
		{
			vertexBufferDesc.ByteWidth = static_cast<UINT>(vertices.size()) * sizeof(TextVertex);
			vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
			vertexBufferDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;
		}

		D3D11_SUBRESOURCE_DATA subResData = {};
		{
			subResData.pSysMem = vertices.data();
		}

		HRESULT result = myDevice->CreateBuffer(&vertexBufferDesc, &subResData, &vertexBuffer);

		if (FAILED(result))
		{
			LOG_ERROR(LogType::Renderer) << "CreateBuffer failed";
		}
	}

	ID3D11Buffer* indexBuffer = nullptr;
	{
		D3D11_BUFFER_DESC indexBufferDesc = {};
		{
			indexBufferDesc.ByteWidth = static_cast<UINT>(indices.size()) * sizeof(indices[0]);
			indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
			indexBufferDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_INDEX_BUFFER;
		}

		D3D11_SUBRESOURCE_DATA subResData = {};
		{
			subResData.pSysMem = indices.data();
		}

		HRESULT result = myDevice->CreateBuffer(&indexBufferDesc, &subResData, &indexBuffer);

		if (FAILED(result))
		{
			LOG_ERROR(LogType::Renderer) << "CreateBuffer failed";
		}
	}

	Text::TextData textData;
	{
		textData.myNumbersOfVertices = static_cast<UINT>(vertices.size());
		textData.myVertexBuffer = vertexBuffer;
		textData.myNumberOfIndices = static_cast<UINT>(indices.size());
		textData.myIndexBuffer = indexBuffer;
		textData.myStride = sizeof(TextVertex);
		textData.myOffset = 0;
		textData.myPrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	}

	Text text;
	text.Init(textData, aText, &It->second);

	return text;
}
