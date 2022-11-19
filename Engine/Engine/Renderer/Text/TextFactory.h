#pragma once

#include "Text.h"
#include "Font.h"

namespace Engine
{
	class Text;

	class TextFactory
	{
	public:
		TextFactory(ID3D11Device* aDevice);

		bool Init();

		Text CreateText(const std::string& aText, const float aWorldSize, const bool aIsCentered = false);

	private:
		ID3D11Device* myDevice = nullptr;

		std::vector<Text*> myTextObjects;
		std::unordered_map<std::string, Font> myFonts;
	};
}