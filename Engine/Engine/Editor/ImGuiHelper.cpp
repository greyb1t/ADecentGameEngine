#include "pch.h"
#include "ImGuiHelper.h"
#include "Engine\Renderer\GraphicsEngine.h"

const ImGuiPayload* ImGuiHelper::BeginDragDropTarget(
	const std::string& aType,
	const ImVec4& aHighlightColor,
	const ImVec4& aHoveredColor)
{
	if (const ImGuiPayload* payload = ImGui::GetDragDropPayload())
	{
		if (std::string(payload->DataType) == aType)
		{
			const auto max = ImGui::GetItemRectMax();
			const auto min = ImGui::GetItemRectMin();

			ImU32 color;

			if (ImGui::IsItemHovered(ImGuiHoveredFlags_RectOnly))
			{
				color = ImGui::ColorConvertFloat4ToU32(aHoveredColor);
			}
			else
			{
				color = ImGui::ColorConvertFloat4ToU32(aHighlightColor);
			}

			ImGui::GetWindowDrawList()->AddRectFilled(min, max, color);
		}
	}

	if (ImGui::BeginDragDropTarget())
	{
		const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(aType.c_str());

		ImGui::EndDragDropTarget();

		return payload;
	}

	return nullptr;
}

void ImGuiHelper::AlignedWidget(const std::string& aText, const float aAlignPercent)
{
	ImGui::AlignTextToFramePadding();
	ImGui::Text("%s", aText.c_str());

	ImGui::SameLine(ImGui::GetContentRegionAvailWidth() * aAlignPercent);
}

void ImGuiHelper::AlignedWidget2(const std::string& aText, const float aAlignPercent)
{
	auto cursor = ImGui::GetCursorPos();

	// Aligns the text vertially to be center in the table
	ImGui::AlignTextToFramePadding();
	ImGui::Text("%s", aText.c_str());

	// This dummy is used to align the components
	ImGui::SetCursorPos(cursor);
	ImGui::Dummy(ImVec2(ImGui::GetWindowWidth() * aAlignPercent, 0));

	ImGui::SameLine(/*ImGui::GetContentRegionAvailWidth() * Editor::ourAlignPercent*/);

	ImGui::SetNextItemWidth(ImGui::GetContentRegionAvailWidth());
}

void ImGuiHelper::CenterNextWindow(const ImGuiCond aCond)
{
	ImGui::SetNextWindowPos(ImVec2(
		ImGui::GetIO().DisplaySize.x * 0.5f,
		ImGui::GetIO().DisplaySize.y * 0.5f), aCond, ImVec2(0.5f, 0.5f));
}

void ImGuiHelper::TextCentered(const std::string& aText)
{
	ImVec2 textSize = ImGui::CalcTextSize(aText.c_str());
	auto size = ImGui::GetItemRectSize();
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (size.x - textSize.x) * 0.5f);
	ImGui::Text(aText.c_str());
}

void ImGuiHelper::TextCenteredVertical(const std::string& aText)
{
	ImVec2 textSize = ImGui::CalcTextSize(aText.c_str());
	auto size = ImGui::GetItemRectSize();
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (size.y - textSize.y) * 0.5f);
	ImGui::Text(aText.c_str());
}

ImVec4 ImGuiHelper::CalculateColorFromString(const std::string& aString)
{
	std::hash<std::string> hasher;
	auto hash = hasher(aString);

	const auto& defaultHeaderColor = ImGui::GetStyleColorVec4(ImGuiCol_Header);

	const float f = 0.05f;

	Vec3f colorsModifiers[] =
	{
		Vec3f(f, 0.f, 0.f),
		Vec3f(0.f, f, 0.f),
		Vec3f(0.f, 0.f, f),

		Vec3f(-f, 0.f, 0.f),
		Vec3f(0.f, -f, 0.f),
		Vec3f(0.f, 0.f, -f),

		Vec3f(-f, f, 0.f),
		Vec3f(0.f, -f, f),
		Vec3f(f, 0.f, -f),

		Vec3f(-f, f, f),
		Vec3f(f, -f, f),
		Vec3f(f, f, -f),

		Vec3f(f, -f, -f),
		Vec3f(-f, f, -f),
		Vec3f(-f, -f, f),
	};

	const auto choosenModifier = colorsModifiers[hash % std::size(colorsModifiers)];

	return ImVec4(
		defaultHeaderColor.x + choosenModifier.x,
		defaultHeaderColor.y + choosenModifier.y,
		defaultHeaderColor.z + choosenModifier.z,
		defaultHeaderColor.w);
}

void ImGuiHelper::TextTitle(const std::string& aText)
{
	ImGui::PushFont(Engine::GraphicsEngine::ourFont24);
	ImGui::Text(aText.c_str());
	ImGui::PopFont();
}

bool ImGuiHelper::RedRemoveButton(const char* aLabel, const float aHeight)
{
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 0, 0, 1));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.7f, 0, 0, 1));
	bool ret = ImGui::Button(aLabel, ImVec2(10.f, aHeight));
	ImGui::PopStyleColor(2);
	return ret;
}

bool ImGuiHelper::InputTextHintCheckValidity(
	const char* aLabel,
	const char* aHint,
	std::string& aText,
	std::function<std::string(const std::string& aText)> aIsValidCallback)
{
	std::string errorText;

	bool isValid = true;

	if (aIsValidCallback)
	{
		errorText = aIsValidCallback(aText);
		isValid = errorText.empty();
	}

	if (!isValid)
	{
		ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1, 0, 0, 1));
	}

	ImGui::InputTextWithHint(aLabel, aHint, &aText);

	if (!isValid)
	{
		ImGui::TextColored(ImVec4(1, 0, 0, 1), errorText.c_str());
		ImGui::PopStyleColor();
	}

	return isValid;
}
