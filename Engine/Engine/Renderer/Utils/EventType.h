#pragma once

namespace Engine
{
	struct WindowResizedData
	{
		Vec2f myClientSize;
		Vec2f myWindowSize;
	};

	enum class EventType
	{
		WindowResized, // WindowResizedData newSizes
		DraggedFileIntoWindow, // std::vector<std::string> Paths
	};
}