#pragma once

namespace Engine
{
class DebugMenu
{
public:
	using DebugMenuEntryCallback = std::function<void()>;

	static void RenderImgui();

	static void AddMenuEntry(
		const std::string& aMenuName, DebugMenuEntryCallback aImguiDrawCallback, void* aId = nullptr);

	static void ToggleDebugMenu(const bool aEnabled);
	static bool Enabled();

private:
	struct DebugMenuEntry
	{
		DebugMenuEntryCallback myCallback;
		std::string myName;
		void* myId = nullptr;
	};

private:
	inline static std::vector<DebugMenuEntry> ourDebugEntryCallbacks;
	inline static bool ourIsDebugMenuActive = false;
};
}
