#pragma once

#include "../RTransform.h"

namespace Engine
{
	struct Font;

	class Text
	{
	public:
		struct TextData
		{
			UINT myNumbersOfVertices;
			UINT myNumberOfIndices;
			UINT myStride;
			UINT myOffset;
			ID3D11Buffer* myVertexBuffer = nullptr;
			ID3D11Buffer* myIndexBuffer = nullptr;
			D3D_PRIMITIVE_TOPOLOGY myPrimitiveTopology;
		};

		bool Init(const TextData& aTextData, const std::string& aText, Font* aFont);

		const RTransform& GetTransform() const;
		RTransform& GetTransform();

		const TextData& GetTextData() { return myTextData; }

		Font* GetFont() { return myFont; }

		const C::Vector4f& GetColor() const;
		void SetColor(const C::Vector4f& aColor);

		inline const std::string& GetText() const { return myText; };

		bool& GetIs2D() { return myIs2D; }

		void SetSortOrder(const int aSortOrder);
		int GetSortOrder() const;

	private:
		TextData myTextData = {};
		std::string myText;

		C::Vector4f myColor = {1.f, 1.f, 1.f, 1.f};

		// CU::Matrix4f myTransform;
		RTransform myTransform;

		bool myIs2D = false;

		Font* myFont = nullptr;
		int mySortOrder = 0;
	};
}