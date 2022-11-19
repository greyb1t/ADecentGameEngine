#pragma once

#include "WindowHandler.h"
#include "Engine/ResourceReferences.h"
#include "Statistics\RenderingStatistics.h"

class FileWatcher;
struct ImFont;

namespace Engine
{
	class ResourceManager;
	struct ResourceManagerSettings;
}

namespace Engine
{
	class RenderManager;
	class RendererScene;
	class FullscreenTextureFactory;
	class ResourceThreadContext;
	class ModelFactory;
	class ShaderLoader;
	class TextureLoader;
	class MaterialFactory;
	class AnimationClipLoader;
	class Directx11Framework;
	class CameraFactory;
	class FullscreenEffectManager;

	class GraphicsEngine
	{
	public:
		GraphicsEngine(FileWatcher* aFileWatcher);

		~GraphicsEngine();

		bool InitFramework(const WindowData& aWindowData);
		bool Init(
			const WindowData& aWindowData,
			ResourceManager& aResourceManager,
			ResourceReferences& aResourceReferences);

		void DrawDebugMenu();

		void Update();

		void BeginFrame();
		void EndFrame();
		void BeginFrameImgui();
		void ImGuiNewFrame();
		void ImGuiRenderFrame();
		void RenderFrame(RendererScene& aScene, RendererScene* aUIScene);
		void EndRenderFrame(RendererScene& aScene);

		const Directx11Framework& GetDxFramework() const;
		const WindowHandler& GetWindowHandler() const;
		WindowHandler& GetWindowHandler();

		RenderManager& GetRenderManager();
		JsonManager& GetJsonManager();
		MaterialFactory& GetMaterialFactory();
		ModelFactory& GetModelFactory();
		CameraFactory& GetCameraFactory();
		FullscreenTextureFactory& GetFullscreenTextureFactory();

	private:
		bool InitImgui();
		void CleanupImgui();

	private:
		Owned<Directx11Framework> myFramework;
		Owned<WindowHandler> myWindowHandler;

		Owned<RenderManager> myRenderManager = nullptr;

		Owned<ModelFactory> myModelFactory = nullptr;
		Owned<FullscreenTextureFactory> myFullscreenTextureFactory = nullptr;
		Owned<MaterialFactory> myMaterialFactory = nullptr;
		// Owned<AnimationClipLoader> myAnimationFactory = nullptr;
		Owned<CameraFactory> myCameraFactory;

		// NOTE: Now we have a duplicate JsonManager in Engine and here in GraphicsEngine
		// Is this okay, or should we send the one in Engine to here by argument instead?
		Owned<JsonManager> myJsonManager = nullptr;

		FileWatcher* myFileWatcher = nullptr;

		ComPtr<ID3D11Buffer> myCubemapConstantBuffer;

		struct CubemapBufferData
		{
			float myRoughness;
			Vec3f myTrash;
		} myCubemapData = { };

	public:
		static inline ImFont* ourFont16 = nullptr;
		static inline ImFont* ourFont16Bold = nullptr;
		static inline ImFont* ourFont24 = nullptr;
		static inline ImFont* ourFont24Bold = nullptr;
		static inline ImFont* ourFont36 = nullptr;
	};
}