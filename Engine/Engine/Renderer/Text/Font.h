#pragma once

namespace Engine
{
	struct Font
	{
		struct Atlas
		{
			int Size; // should be float as in json?
			int Width;
			int Height;
			float EmSize;
			float LineHeight;
			float Ascender;
			float Descender;
			float UnderlineY;
			float UnderlineThickness;
		} Atlas;

		struct Glyph
		{
			char Character;
			float Advance;
			C::Vector4f PlaneBounds;
			C::Vector4f UVBounds;
		};

		std::unordered_map<unsigned int, Glyph> Glyphs;

		ID3D11ShaderResourceView* SRV;

		Glyph operator[](char c);
	};
}