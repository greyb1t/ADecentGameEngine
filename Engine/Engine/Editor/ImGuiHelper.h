#pragma once

namespace ImGuiHelper
{
	const ImGuiPayload* BeginDragDropTarget(
		const std::string& aType,
		const ImVec4& aHighlightColor = ImVec4(0, 1, 0, 0.2),
		const ImVec4& aHoveredColor = ImVec4(0, 1, 0, 0.5));

	// Called before a normal widget, this draws a text and then leaves place
	// for the following widget on the same line by align percent
	void AlignedWidget(const std::string& aText, const float aAlignPercent);

	// The following item will be drawn with avail X width
	void AlignedWidget2(const std::string& aText, const float aAlignPercent);

	void CenterNextWindow(const ImGuiCond aCond);

	void TextCentered(const std::string& aText);
	void TextCenteredVertical(const std::string& aText);

	ImVec4 CalculateColorFromString(const std::string& aString);

	void TextTitle(const std::string& aText);

	bool RedRemoveButton(const char* aLabel, const float aHeight);

	// Returns true if valid, false if not
	bool InputTextHintCheckValidity(
		const char* aLabel,
		const char* aHint,
		std::string& aText,
		std::function<std::string(const std::string& aText)> aIsValidCallback);
}

namespace ImGui
{
	//Usable only with BETTER_ENUM:s
	template<typename T>
	inline void EnumChoice(const std::string& aHeader, T& aBetterEnum)
	{
		if (ImGui::BeginCombo(aHeader.c_str(), aBetterEnum._to_string()))
		{
			for (unsigned i = 0; i < T::_size(); ++i)
			{
				T iEnum(T::_from_index(i));
				if (ImGui::Selectable(iEnum._to_string(), aBetterEnum == iEnum))
				{
					aBetterEnum = iEnum;
				}
			}
			ImGui::EndCombo();
		}
	}
}