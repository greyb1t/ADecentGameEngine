#pragma once

#include "CreationState.h"
#include "Engine\ResourceManagement\ResourceRef.h"

namespace Engine
{
	class AnimationClip;
}

class GameObject;

namespace Engine
{
	class Resource;
	class Editor;

	class AnimationCreationState : public CreationState
	{
	public:
		struct AnimationClipEntry
		{
			Path myAnimationClipPath;
			Owned<AnimationClip> myClip;
		};

		AnimationCreationState(Editor& aEditor);
		AnimationCreationState(const std::filesystem::path& aPath, Editor& aEditor);
		~AnimationCreationState();

		bool Draw() override;

		void ParseFiles(const std::vector<std::filesystem::path>& aPaths) override;

		friend class ModelTextureResource;

	private:
		ImVec4 GetColorFromPath(const std::string& aPath) const;
		bool ExportAnimationClipEntry(AnimationClipEntry& aClipEntry, const ModelRef& aModel);

	private:
		Editor& myEditor;

		ModelRef myAttachedFbx;
		std::vector<AnimationClipEntry> myAnimationClips;

		std::vector<Owned<Resource>> myResourceToHandle;
	};
}