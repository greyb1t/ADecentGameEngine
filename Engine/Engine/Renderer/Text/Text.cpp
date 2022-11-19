#include "pch.h"
#include "Text.h"

bool Engine::Text::Init(const TextData& aTextData, const std::string& aText, Font* aFont)
{
	myTextData = aTextData;
	myText = aText;
	myFont = aFont;

	return true;
}

Engine::RTransform& Engine::Text::GetTransform()
{
	return myTransform;
}

const C::Vector4f& Engine::Text::GetColor() const
{
	return myColor;
}

void Engine::Text::SetColor(const C::Vector4f& aColor)
{
	myColor = aColor;
}

void Engine::Text::SetSortOrder(const int aSortOrder)
{
	mySortOrder = aSortOrder;
}

int Engine::Text::GetSortOrder() const
{
	return mySortOrder;
}

const Engine::RTransform& Engine::Text::GetTransform() const
{
	return myTransform;
}
