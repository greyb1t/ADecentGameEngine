#pragma once

enum class DebugDrawFlags
{
	Pointlights,
	Spotlights,
	EnvironmentLight,
	BoundingSpheres,
	Navmesh,
	Frustum,
	UI,
	Physics,
	Particles,
	Gameplay,
	AI,
	Player,
	Triggers,
	Bezier,
	Always, // Means it always gets drawn, no exception, even when debug drawing is disabled

	Count
};

namespace Engine
{

	std::string DebugDrawFlagToString(const DebugDrawFlags aFlag);

	class DebugDrawerSettings
	{
	public:
		void Update();

		bool LoadDebugDrawSettings();
		bool SaveDebugDrawSettings();

		void DrawDebugMenu();

		bool IsDebugFlagSet(const DebugDrawFlags aFlag) const;
		void SetDebugFlag(const DebugDrawFlags aFlag, const bool aEnabled);

		bool IsDebugFlagActive(const DebugDrawFlags aFlag) const;

		bool IsActive() const;

	private:
		bool myActive = false;
		std::bitset<static_cast<int>(DebugDrawFlags::Count)> myFlags;
	};
}