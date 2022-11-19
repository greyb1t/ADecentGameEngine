#pragma once

namespace Engine
{
	class NormalAnimation;

	struct AnimationEvent
	{
	public:
		enum class Type
		{
			ByFrame,
			ByNormalizedTime
		};

		static inline const char* ourTypes[] =
		{
			"By Frame",
			"By Normalized Time"
		};

		bool InitFromJson(const nlohmann::json& aJson);
		nlohmann::json ToJson() const;

		const std::string GetName() const;

		bool WasExecuted(const NormalAnimation& aNormalAnimation) const;

	private:
		friend class AnimationNodeEditorWindow;

		Type myType = Type::ByFrame;

		std::string myEventName;

		int myFrame = 0;
		float myNormalizedTime = 0.f;

		bool myHasExecutedThisLoopIteration = false;
	};
}