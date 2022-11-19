#include "pch.h"
#include "GraphicsEngine.h"
#include "Model/ModelInstance.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Renderer/Texture/FullscreenTextureFactory.h"
#include "RenderManager.h"

#include "Engine/Renderer/Model/ModelFactory.h"
#include "Engine/Renderer/Shaders/ShaderLoader.h"
#include "Engine/Renderer/Texture/TextureLoader.h"
#include "Engine/Renderer/Material/MaterialFactory.h"
#include "Engine/Renderer/Animation/AnimationClipLoader.h"
#include "Engine/ResourceManagement/ResourceManager.h"
#include "Directx11Framework.h"
#include "Camera/CameraFactory.h"
#include "Engine/GameObject/Components/ReflectionCaptureComponent.h"
#include "Utils/DxUtils.h"
#include "FrameConstantBuffer.h"
#include "Engine/ResourceManagement/Resources/ModelResource.h"
#include "Engine/Renderer/Model/Model.h"
#include "Engine/ResourceManagement/Resources/PixelShaderResource.h"
#include "Engine/ResourceManagement/Resources/VertexShaderResource.h"
#include "Engine/Renderer/Shaders/VertexShader.h"
#include "Engine/Renderer/Shaders/PixelShader.h"
#include "Scene/RendererScene.h"
#include "Engine/GameObject/Components/EnvironmentLightComponent.h"
#include "Engine/Editor/Editor.h"
#include "Engine/Paths.h"

namespace Engine
{
	GraphicsEngine::GraphicsEngine(FileWatcher* aFileWatcher)
		: myFileWatcher(aFileWatcher)
	{
	}

	GraphicsEngine::~GraphicsEngine()
	{
		// Manually delete them in proper order

		myFramework = nullptr;
		myWindowHandler = nullptr;

		{
			// Must be destroyed after resource manager

			myModelFactory = nullptr;
			myMaterialFactory = nullptr;
		}

		myRenderManager = nullptr;
		myFullscreenTextureFactory = nullptr;

		CleanupImgui();
	}

	bool GraphicsEngine::InitFramework(const WindowData& aWindowData)
	{
		myWindowHandler = MakeOwned<WindowHandler>();

		myFramework = MakeOwned<Directx11Framework>(*myWindowHandler);

		if (!myWindowHandler->Init(aWindowData, *myFramework))
		{
			LOG_ERROR(LogType::Renderer) << "Window handler init failed";
			return false;
		}

		if (!myFramework->Init())
		{
			LOG_ERROR(LogType::Renderer) << "Directx11 framework init failed";
			return false;
		}

		return true;
	}

	bool GraphicsEngine::Init(
		const WindowData& aWindowData,
		ResourceManager& aResourceManager,
		ResourceReferences& aResourceReferences)
	{
		if (!InitImgui())
		{
			LOG_ERROR(LogType::Renderer) << "Imgui init failed";
			return false;
		}

		myFullscreenTextureFactory = MakeOwned<FullscreenTextureFactory>();

		myRenderManager = MakeOwned<RenderManager>(
			aResourceManager,
			aResourceReferences,
			*myFullscreenTextureFactory,
			*myWindowHandler,
			*myFramework);

		// TODO: At the moment we have duplicate JsonManager in Engine and here in GraphicsEngine
		// is this a good thing? no lol
		myJsonManager = MakeOwned<JsonManager>();
		myJsonManager->SetFileWatcher(*myFileWatcher);

		myModelFactory = MakeOwned<ModelFactory>(aResourceManager);

		MaterialFactoryContext matFactoryContext;
		matFactoryContext.myDevice = myFramework->GetDevice();
		matFactoryContext.myResourceManager = &aResourceManager;

		myMaterialFactory = MakeOwned<MaterialFactory>(*myJsonManager, matFactoryContext);

		myCameraFactory = MakeOwned<CameraFactory>(*myWindowHandler);

		if (!myFullscreenTextureFactory->Init(myFramework->GetDevice(), myFramework->GetContext()))
		{
			LOG_ERROR(LogType::Renderer) << "Fullscreen texture factory init failed";
			return false;
		}

		if (!myRenderManager->Init(*myFramework))
		{
			LOG_ERROR(LogType::Renderer) << "RenderManager init failed";
			return false;
		}

		if (!myModelFactory->Init(myFramework->GetDevice()))
		{
			LOG_ERROR(LogType::Renderer) << "ModelFactory init failed";
			return false;
		}

		if (!myMaterialFactory->Init())
		{
			LOG_ERROR(LogType::Renderer) << "Material factory failed to init";
			return false;
		}

		return true;
	}

	void GraphicsEngine::DrawDebugMenu()
	{
		myWindowHandler->DrawDebugMenu();

		myRenderManager->DrawDebugMenu();
	}

	void GraphicsEngine::Update()
	{
		myWindowHandler->Update();
	}

	void GraphicsEngine::BeginFrame()
	{
		ZoneScopedN("GraphicsEngine::BeginFrame");

		myFramework->BeginFrame({ 0.f, 162.f / 255.f, 232.f / 255.f, 1 });

		// BeginFrameImgui();
	}

	void GraphicsEngine::EndFrame()
	{
		ZoneScopedN("GraphicsEngine::EndFrame");

		// EndFrameImgui();

		myFramework->EndFrame();
	}

	void GraphicsEngine::RenderFrame(RendererScene& aScene, RendererScene* aUIScene)
	{
		ZoneScopedN("GraphicsEngine::RenderFrame");

		// TODO: Make a camera that we input into RenderScene() instead
		// that contains all that it needs to render and its post processing data

#if 1
		for (const ReflectionCapture& capture : aScene.GetReflectionCaptures())
		{
			capture.myCubemapTexture->ClearTextureWholeCube();

			for (int i = 0; i < 6; ++i)
			{
				const auto& camSide = capture.myCameraSides[i];

				camSide.myCamera->SetPosition(capture.myPosition);

				// TODO: In the future the camera will contain the cubemap it will
				// render with, however, it does not yet
				// therefore, manually set the cubemap when rendering the
				// scene with the reflection capture camera to avoid an
				// recursive feedback loop
				//TextureRef mainCubemap = aScene.GetEnvironmentalLight()->GetCubemap();

				// aScene.GetEnvironmentalLight()->SetCubemap1();

				// TODO: Render the scene to the same resolution as the reflection cubemap render target, otherwise wasted performance
				myRenderManager->RenderScene(aScene, *camSide.myCamera, false, camSide.myCulledSceneData);

				// reset the cubemap back to its original value
				//aScene.GetEnvironmentalLight()->SetCubemap1(mainCubemap);

				capture.myCubemapTexture->SetAsActiveTargetSide(i, nullptr);
				myRenderManager->EndRender2();
			}

			// unbind because we use this cubemap below to blur it
			DxUtils::UnbindRenderTargetView(*myFramework->GetContext());

			// Must do this when creating the actual cubemap texture
			// to allocate the proper amount of memory
			// also here after rendering the scene to the cubemap
			// the prefilter shader uses SampleLevel
			myFramework->GetContext()->GenerateMips(capture.myCubemapTexture->GetCubeSRV());

			// Must go through each mip level and blur it

			// TODO: Make this resolution an option from the component
			// capture.myCubemapTextureSmooth->Init({ 128, 128 });

			const int mipCount = capture.myCubemapTextureSmooth->GetMipLevelCount();

			for (int mipLevel = 0; mipLevel < mipCount; ++mipLevel)
			{
				const int cubeSideCount = 6;

				float roughness = (float)mipLevel / (float)(mipCount - 1);
				// roughness = 1337.f;

				for (int i = 0; i < cubeSideCount; ++i)
				{
					const auto& mip = capture.myCubemapTextureSmooth->GetMipLevel(mipLevel);

					mip.mySideRTVs;
					mip.myViewport;

					auto& context = *myFramework->GetContext();

					context.OMSetRenderTargets(1, mip.mySideRTVs[i].GetAddressOf(), nullptr);

					context.RSSetViewports(1, &mip.myViewport);

					FLOAT color[4] = { 0.f, 0.f, 0.f, 0.f };
					context.ClearRenderTargetView(mip.mySideRTVs[i].Get(), color);

					context.PSSetShaderResources(0, 1, capture.myCubemapTexture->GetCubeSRVPtr());

					myRenderManager->SetSamplerState(0, SamplerState_TrilinearWrap);

					if (myCubemapConstantBuffer == nullptr)
					{
						D3D11_BUFFER_DESC bufDesc = {};
						{
							bufDesc.Usage = D3D11_USAGE::D3D11_USAGE_DYNAMIC;
							bufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
							bufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
						}

						bufDesc.ByteWidth = sizeof(CubemapBufferData);

						HRESULT result = myFramework->GetDevice()->CreateBuffer(&bufDesc, nullptr, &myCubemapConstantBuffer);

						if (FAILED(result))
						{
							LOG_ERROR(LogType::Renderer) << "CreateBuffer failed to create frame cbuffer";
						}
					}

					myCubemapData.myRoughness = roughness;

					{
						D3D11_MAPPED_SUBRESOURCE bufferData;
						ZeroMemory(&bufferData, sizeof(bufferData));

						HRESULT result = context.Map(myCubemapConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &bufferData);
						ThrowIfFailed(result);

						memcpy(bufferData.pData, &myCubemapData, sizeof(myCubemapData));

						context.Unmap(myCubemapConstantBuffer.Get(), 0);
					}

					// Must not use slot 0, since framebuffer uses that slot
					context.PSSetConstantBuffers(2, 1, myCubemapConstantBuffer.GetAddressOf());

					// is this really good enough?
					myRenderManager->GetFrameBuffer().UpdateCamera(*capture.myCameraSides[i].myCamera);

					myRenderManager->GetFrameBuffer().Bind();

					myRenderManager->SetRasterizerState(RasterizerState_NoCulling);

					myRenderManager->SetDepthStencilState(DepthStencilState_Default);

					auto vs =
						GResourceManager->CreateRef<VertexShaderResource>(
							"Assets/Shaders/Engine/Cubemap/SkyboxVS",
							DefaultVertex::ourInputElements,
							DefaultVertex::ourElementCount);
					vs->Load();
					assert(vs->IsValid());

					auto ps = GResourceManager->CreateRef<PixelShaderResource>(
						"Assets/Shaders/Engine/Cubemap/PrefilterCubemapPS");
					ps->Load();
					assert(ps->IsValid());

					context.IASetInputLayout(vs->Get().GetInputLayout());

					context.VSSetShader(vs->Get().GetVertexShader(), nullptr, 0);
					context.PSSetShader(ps->Get().GetPixelShader(), nullptr, 0);

					//auto cube = GResourceManager->CreateRef<ModelResource>("Assets\\Test\\chrome_cube\\ChromeCube.model");
					auto cube = GResourceManager->CreateRef<ModelResource>("Assets/Engine/Meshes/Cube1CM.model");
					cube->Load();
					assert(cube->IsValid());

					auto vertexBuffer = cube->Get().GetVertexBuffer();
					auto indexBuffer = cube->Get().GetIndexBuffer();
					auto stride = cube->Get().GetStride();
					auto offset = cube->Get().GetOffset();

					context.IASetVertexBuffers(0, 1, &vertexBuffer, stride, offset);

					context.IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

					context.IASetPrimitiveTopology(cube->Get().myPrimitiveTopology);

					// assumes cube and 1 mesh
					context.DrawIndexed(
						cube->Get().myMeshDatas[0].myIndicesCount,
						cube->Get().myMeshDatas[0].myStartIndexLocation,
						cube->Get().myMeshDatas[0].myBaseVertexLocation);

					// FILIP:
					/*
						Du kollade senast i graphics debuggern på varje cube side
						som renderas med nya shadern, de 2 första såg bra ut
						sedan blev det weird
					*/
					// assert(false);
				}
			}

			// unbind because we use this the newly blurred cubemap
			DxUtils::UnbindRenderTargetView(*myFramework->GetContext());



			/*
				for each mip level
				{
					for each camera side
					{
						blur each side texture
					}
				}
			*/
		}
#endif 

		const auto stats = myRenderManager->RenderScene(
			aScene,
			*aScene.GetMainCamera(),
			false,
			aScene.GetCulledSceneData());

		aScene.SetStats(stats);

		for (const auto& submittedDxCall : aScene.GetSubmittedDXCalls())
		{
			submittedDxCall();
		}
	}

	void GraphicsEngine::EndRenderFrame(RendererScene& aScene)
	{
		myRenderManager->EndRender(aScene);
	}

	const Directx11Framework& GraphicsEngine::GetDxFramework() const
	{
		return *myFramework;
	}

	const WindowHandler& GraphicsEngine::GetWindowHandler() const
	{
		return *myWindowHandler;
	}

	WindowHandler& GraphicsEngine::GetWindowHandler()
	{
		return *myWindowHandler;
	}

	RenderManager& GraphicsEngine::GetRenderManager()
	{
		return *myRenderManager;
	}

	JsonManager& GraphicsEngine::GetJsonManager()
	{
		return *myJsonManager;
	}

	MaterialFactory& GraphicsEngine::GetMaterialFactory()
	{
		return *myMaterialFactory;
	}

	ModelFactory& GraphicsEngine::GetModelFactory()
	{
		return *myModelFactory;
	}

	CameraFactory& GraphicsEngine::GetCameraFactory()
	{
		return *myCameraFactory;
	}

	FullscreenTextureFactory& GraphicsEngine::GetFullscreenTextureFactory()
	{
		return *myFullscreenTextureFactory;
	}

	bool GraphicsEngine::InitImgui()
	{
		IMGUI_CHECKVERSION();

		ImGui::CreateContext();
		ImGui::StyleColorsDark();

		const auto oldIniPath = "Dev/imgui.ini";
		static std::string iniPath = (GetEngine().GetPaths().GetPath(PathEnum::EditorSaveFolder) / "imgui.ini").string();

		if (!std::filesystem::exists(GetEngine().GetPaths().GetPath(PathEnum::EditorSaveFolder)))
		{
			std::filesystem::create_directory(GetEngine().GetPaths().GetPath(PathEnum::EditorSaveFolder));
		}

		if (std::filesystem::exists(oldIniPath))
		{
			if (!std::filesystem::exists(GetEngine().GetPaths().GetPath(PathEnum::EditorSaveFolder)))
			{
				std::filesystem::create_directory(GetEngine().GetPaths().GetPath(PathEnum::EditorSaveFolder));
			}
			std::error_code e;

			FileIO::RemoveReadOnly(Path(oldIniPath));

			// Copy to destination
			std::filesystem::copy(
				oldIniPath,
				iniPath,
				std::filesystem::copy_options::recursive |
				std::filesystem::copy_options::overwrite_existing,
				e);

			if (e.value() != 0)
			{
				LOG_ERROR(LogType::Editor) << "Unable to copy old imgui ini to new path!";
				return false;
			}

			std::filesystem::remove_all(oldIniPath, e);

			if (e.value() != 0)
			{
				LOG_ERROR(LogType::Editor) << "Unable to remove old save path folder!";
				return false;
			}
		}

		auto& io = ImGui::GetIO();
		io.IniFilename = iniPath.c_str();
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

		ourFont16 = io.Fonts->AddFontFromFileTTF("Assets\\Engine\\Editor\\Fonts\\Roboto-Regular.ttf", 16.0f);
		ourFont16Bold = io.Fonts->AddFontFromFileTTF("Assets\\Engine\\Editor\\Fonts\\Roboto-Bold.ttf", 16.0f);
		ourFont24 = io.Fonts->AddFontFromFileTTF("Assets\\Engine\\Editor\\Fonts\\Roboto-Regular.ttf", 24.0f);
		ourFont24Bold = io.Fonts->AddFontFromFileTTF("Assets\\Engine\\Editor\\Fonts\\Roboto-Bold.ttf", 24.0f);
		ourFont36 = io.Fonts->AddFontFromFileTTF("Assets\\Engine\\Editor\\Fonts\\Roboto-Regular.ttf", 36.0f);

		io.KeyMap[ImGuiKey_Tab] = VK_TAB;
		io.KeyMap[ImGuiKey_LeftArrow] = VK_LEFT;
		io.KeyMap[ImGuiKey_RightArrow] = VK_RIGHT;
		io.KeyMap[ImGuiKey_UpArrow] = VK_UP;
		io.KeyMap[ImGuiKey_DownArrow] = VK_DOWN;
		io.KeyMap[ImGuiKey_PageUp] = VK_PRIOR;
		io.KeyMap[ImGuiKey_PageDown] = VK_NEXT;
		io.KeyMap[ImGuiKey_Home] = VK_HOME;
		io.KeyMap[ImGuiKey_End] = VK_END;
		io.KeyMap[ImGuiKey_Insert] = VK_INSERT;
		io.KeyMap[ImGuiKey_Delete] = VK_DELETE;
		io.KeyMap[ImGuiKey_Backspace] = VK_BACK;
		io.KeyMap[ImGuiKey_Space] = VK_SPACE;
		io.KeyMap[ImGuiKey_Enter] = VK_RETURN;
		io.KeyMap[ImGuiKey_Escape] = VK_ESCAPE;
		io.KeyMap[ImGuiKey_KeyPadEnter] = VK_RETURN;
		io.KeyMap[ImGuiKey_LeftControl] = VK_CONTROL;
		io.KeyMap[ImGuiKey_LeftShift] = VK_SHIFT;
		io.KeyMap[ImGuiKey_A] = 'A';
		io.KeyMap[ImGuiKey_B] = 'B';
		io.KeyMap[ImGuiKey_C] = 'C';
		io.KeyMap[ImGuiKey_D] = 'D';
		io.KeyMap[ImGuiKey_E] = 'E';
		io.KeyMap[ImGuiKey_F] = 'F';
		io.KeyMap[ImGuiKey_G] = 'G';
		io.KeyMap[ImGuiKey_H] = 'H';
		io.KeyMap[ImGuiKey_I] = 'I';
		io.KeyMap[ImGuiKey_J] = 'J';
		io.KeyMap[ImGuiKey_K] = 'K';
		io.KeyMap[ImGuiKey_L] = 'L';
		io.KeyMap[ImGuiKey_M] = 'M';
		io.KeyMap[ImGuiKey_N] = 'N';
		io.KeyMap[ImGuiKey_O] = 'O';
		io.KeyMap[ImGuiKey_P] = 'P';
		io.KeyMap[ImGuiKey_Q] = 'Q';
		io.KeyMap[ImGuiKey_R] = 'R';
		io.KeyMap[ImGuiKey_S] = 'S';
		io.KeyMap[ImGuiKey_V] = 'V';
		io.KeyMap[ImGuiKey_X] = 'X';
		io.KeyMap[ImGuiKey_Y] = 'Y';
		io.KeyMap[ImGuiKey_Z] = 'Z';
		io.KeyMap[ImGuiKey_F1] = VK_F1;
		io.KeyMap[ImGuiKey_F2] = VK_F2;
		io.KeyMap[ImGuiKey_F3] = VK_F3;
		io.KeyMap[ImGuiKey_F4] = VK_F4;
		io.KeyMap[ImGuiKey_F5] = VK_F5;
		io.KeyMap[ImGuiKey_F6] = VK_F6;
		io.KeyMap[ImGuiKey_F7] = VK_F7;
		io.KeyMap[ImGuiKey_F8] = VK_F8;
		io.KeyMap[ImGuiKey_F9] = VK_F9;
		io.KeyMap[ImGuiKey_F10] = VK_F10;
		io.KeyMap[ImGuiKey_F11] = VK_F11;
		io.KeyMap[ImGuiKey_F12] = VK_F12;

		constexpr auto ColorFromBytes = [](uint8_t r, uint8_t g, uint8_t b)
		{
			return ImVec4((float)r / 255.0f, (float)g / 255.0f, (float)b / 255.0f, 1.0f);
		};

		auto& style = ImGui::GetStyle();
		ImVec4* colors = style.Colors;

		const ImVec4 bgColor = ColorFromBytes(37, 37, 38);
		const ImVec4 lightBgColor = ColorFromBytes(82, 82, 85);
		const ImVec4 veryLightBgColor = ColorFromBytes(90, 90, 95);

		const ImVec4 panelColor = ColorFromBytes(51, 51, 55);
		const ImVec4 panelHoverColor = ColorFromBytes(29, 151, 236);
		const ImVec4 panelActiveColor = ColorFromBytes(0, 119, 200);

		const ImVec4 textColor = ColorFromBytes(255, 255, 255);
		const ImVec4 textDisabledColor = ColorFromBytes(151, 151, 151);
		const ImVec4 borderColor = ColorFromBytes(78, 78, 78);

		colors[ImGuiCol_Text] = textColor;
		colors[ImGuiCol_TextDisabled] = textDisabledColor;
		colors[ImGuiCol_TextSelectedBg] = panelActiveColor;
		colors[ImGuiCol_WindowBg] = bgColor;
		colors[ImGuiCol_ChildBg] = bgColor;
		colors[ImGuiCol_PopupBg] = bgColor;
		colors[ImGuiCol_Border] = ColorFromBytes(70, 70, 70);
		colors[ImGuiCol_BorderShadow] = ColorFromBytes(27, 27, 27);
		colors[ImGuiCol_FrameBg] = ColorFromBytes(53, 53, 56);
		colors[ImGuiCol_FrameBgHovered] = /*panelHoverColor*/ColorFromBytes(91, 91, 91);
		colors[ImGuiCol_FrameBgActive] = ColorFromBytes(120, 120, 120);
		colors[ImGuiCol_TitleBg] = bgColor;
		colors[ImGuiCol_TitleBgActive] = ColorFromBytes(69, 69, 69);
		colors[ImGuiCol_TitleBgCollapsed] = bgColor;
		colors[ImGuiCol_MenuBarBg] = panelColor;
		colors[ImGuiCol_ScrollbarBg] = panelColor;
		colors[ImGuiCol_ScrollbarGrab] = lightBgColor;
		colors[ImGuiCol_ScrollbarGrabHovered] = veryLightBgColor;
		colors[ImGuiCol_ScrollbarGrabActive] = ColorFromBytes(206, 206, 206);
		colors[ImGuiCol_CheckMark] = ColorFromBytes(214, 214, 214);
		colors[ImGuiCol_SliderGrab] = ColorFromBytes(177, 177, 177);
		colors[ImGuiCol_SliderGrabActive] = ColorFromBytes(228, 228, 228);
		colors[ImGuiCol_Button] = panelColor;
		colors[ImGuiCol_ButtonHovered] = ColorFromBytes(104, 104, 104);
		colors[ImGuiCol_ButtonActive] = ColorFromBytes(128, 128, 128);
		colors[ImGuiCol_Header] = ColorFromBytes(81, 81, 85);
		colors[ImGuiCol_HeaderActive] = ColorFromBytes(170, 170, 170);
		colors[ImGuiCol_HeaderHovered] = ColorFromBytes(104, 104, 104);
		colors[ImGuiCol_Separator] = borderColor;
		colors[ImGuiCol_SeparatorHovered] = ColorFromBytes(178, 178, 178);
		colors[ImGuiCol_SeparatorActive] = ColorFromBytes(255, 255, 255);
		colors[ImGuiCol_ResizeGrip] = ColorFromBytes(80, 80, 80);
		colors[ImGuiCol_ResizeGripHovered] = ColorFromBytes(120, 120, 120);
		colors[ImGuiCol_ResizeGripActive] = ColorFromBytes(255, 255, 255);
		colors[ImGuiCol_PlotLines] = panelActiveColor;
		colors[ImGuiCol_PlotLinesHovered] = panelHoverColor;
		colors[ImGuiCol_PlotHistogram] = panelActiveColor;
		colors[ImGuiCol_PlotHistogramHovered] = panelHoverColor;
		colors[ImGuiCol_DragDropTarget] = bgColor;
		colors[ImGuiCol_NavHighlight] = bgColor;
		colors[ImGuiCol_DockingPreview] = ColorFromBytes(106, 106, 106);
		colors[ImGuiCol_Tab] = bgColor;
		colors[ImGuiCol_TabActive] = ColorFromBytes(128, 128, 128);
		colors[ImGuiCol_TabUnfocused] = bgColor;
		colors[ImGuiCol_TabUnfocusedActive] = ColorFromBytes(70, 70, 70);
		colors[ImGuiCol_TabHovered] = ColorFromBytes(161, 161, 161);

		// Main
		style.WindowMinSize = ImVec2(100.f, 100.f);

		style.WindowPadding = ImVec2(10.f, 10.f);
		style.FramePadding = ImVec2(12.f, 4.f);
		style.CellPadding = ImVec2(10.f, 8.f);
		style.ItemSpacing = ImVec2(8.f, 6.f);
		style.ItemInnerSpacing = ImVec2(8.f, 4.f);
		style.TouchExtraPadding = ImVec2(0.f, 0.f);
		style.IndentSpacing = 21.f;
		style.ScrollbarSize = 18.f;
		style.GrabMinSize = 10.f;

		// Borders
		style.WindowBorderSize = 1.f;
		style.ChildBorderSize = 1.f;
		style.PopupBorderSize = 1.f;
		style.FrameBorderSize = 1.f;
		style.TabBorderSize = 0.f;

		// Rounding
		style.WindowRounding = 0.f;
		style.ChildRounding = 0.f;
		style.FrameRounding = 0.f;
		style.PopupRounding = 0.f;
		style.ScrollbarRounding = 12.f;
		style.GrabRounding = 0.f;
		style.LogSliderDeadzone = 4.f;
		style.TabRounding = 0.f;

		// Alignment
		style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
		style.WindowMenuButtonPosition = ImGuiDir_Left;
		style.ColorButtonPosition = ImGuiDir_Right;
		style.SelectableTextAlign = ImVec2(0.5f, 0.f);

		/*
		style.WindowRounding = 0.0f;
		style.ChildRounding = 0.0f;
		style.FrameRounding = 0.0f;
		style.GrabRounding = 0.0f;
		style.PopupRounding = 0.0f;
		style.ScrollbarRounding = 0.0f;
		style.TabRounding = 0.0f;
		*/

		//// gamma-correct style
		//const auto ToLinear = [](float x)
		//{
		//	//return x;
		//	//return std::pow(abs(x), 2.2f);
		//	//return std::pow(abs(x), 1.0f / 2.2f);

		//	if (x <= 0.04045f)
		//	{
		//		return x / 12.92f;
		//	}
		//	else
		//	{
		//		return std::pow((x + 0.055f) / 1.055f, 2.4f);
		//	}
		//};
		//for (int i = 0; i < ImGuiCol_COUNT; i++)
		//{
		//	auto& c = ImGui::GetStyle().Colors[i];
		//	c = { ToLinear(c.x), ToLinear(c.y), ToLinear(c.z), c.w };
		//}

		if (!ImGui_ImplWin32_Init(myWindowHandler->GetWindowHandle()))
		{
			LOG_ERROR(LogType::Renderer) << "ImGui_ImplWin32_Init failed";
			return false;
		}

		if (!ImGui_ImplDX11_Init(myFramework->GetDevice(), myFramework->GetContext()))
		{
			LOG_ERROR(LogType::Renderer) << "ImGui_ImplDX11_Init failed";
			return false;
		}

		return true;
	}

	void GraphicsEngine::BeginFrameImgui()
	{
		ZoneScopedN("GraphicsEngine::BeginFrameImgui");

		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();

		// ImGui::NewFrame();
		// ImGuizmo::BeginFrame();
	}

	void GraphicsEngine::ImGuiNewFrame()
	{
		ImGui::NewFrame();
		ImGuizmo::BeginFrame();
	}

	void GraphicsEngine::ImGuiRenderFrame()
	{
		ZoneScopedN("GraphicsEngine::ImGuiRenderFrame");

		ImGui::Render();

		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}
	}

	void GraphicsEngine::CleanupImgui()
	{
		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}
}