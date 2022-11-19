#include "pch.h"
#include "Font.h"

Engine::Font::Glyph Engine::Font::operator[](char c)
{
	return Glyphs[c];
}
