#pragma once

namespace Engine
{
	class FolderScene;

	class SceneSerializer
	{
	public:
		virtual ~SceneSerializer() = default;

		virtual void Serialize(FolderScene& aFolderScene, const Path& aPath) = 0;
		virtual bool Deserialize(FolderScene& aFolderScene, const Path& aPath) = 0;
	};
}