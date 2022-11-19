#pragma once

namespace Engine
{
	struct SceneJson
	{
		std::string myName = "Empty";

		bool InitFromJson(const Path& aPath);
	};
}