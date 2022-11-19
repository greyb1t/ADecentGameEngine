#pragma once

#include "Engine/ResourceManagement/ResourceRef.h"

namespace Engine
{
	class BlendMask
	{
	public:
		void Init(const int aBoneCount);
		void Clear();

		bool InitFromJson(const nlohmann::json& aJson);
		nlohmann::json ToJson() const;

		bool IsMaskEnabled() const;

		bool IsEnabled(const int aBoneIndex) const;
		void SetIsEnabled(const int aBoneIndex, const bool aValue);

		bool IsEmpty() const;

	private:
		friend class AnimationNodeEditorWindow;

		// AnimationClipRefmyClipForSkeleton;
		std::vector<bool> myMask;

		bool myIsEnabled = false;
	};
}
