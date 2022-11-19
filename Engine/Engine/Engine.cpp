#include "pch.h"
#include "Engine.h"
#include "Scene/SceneManager.h"

#include "Renderer/GraphicsEngine.h"

#include "Engine/Renderer/Scene/RendererScene.h"
#include "Renderer/Model/ModelFactory.h"
#include "Renderer/Camera/CameraFactory.h"
#include "Renderer/Camera//Camera.h"
#include "Renderer/RenderManager.h"
#include "Renderer/Texture/FullscreenTextureFactory.h"
#include "Renderer/Text/TextFactory.h"
#include "ResourceManagement/ResourceThreadContext.h"
#include "Engine/GameObject/GameObject.h"
#include "Engine/Shortcuts.h"
#include "Engine/Time.h"
#include "Engine/GameObject/Components/SkyboxComponent.h"

#include "Scene/Scene.h"
#include "DebugManager/DebugMenu.h"
#include "DebugManager/DebugDrawer.h"
#include "ResourceManagement/ResourceThreadContext.h"
#include "EngineSettings.h"

#include "TimeSystem/TimeSystem.h"
#include "Physics/Physics.h"
#include "ResourceManagement/ResourceManagerSettings.h"
#include "GameObject/ComponentSystem/ComponentRegistrator.h"

#include "Renderer/Directx11Framework.h"
#include "EngineOrder.h"
#include "Renderer/Renderers/DeferredRenderer.h"
#include "Renderer/Text/TextRenderer.h"
#include "Renderer/Renderers/ForwardRenderer.h"
#include "Renderer/Statistics/RenderingStatistics.h"
#include "ResourceManagement/ResourceManager.h"
#include "CameraShake/CameraShakeManager.h"
#include "GameObject/Components/CameraShakeComponent.h"
#include "AnimationCurve/CurveManager.h"
#include "Console/CommandHandler.h"
#include "Console/IngameConsole.h"
#include "AudioManager.h"
#include "TimeStamp.h"
#include "Editor/Editor.h"
#include "GraphManager/GraphNodeRegistry.h"
#include "Scene/FolderScene.h"
#include "Scene/SceneRegistry.h"
#include "Navmesh/NavmeshComponent.h"
#include "ResourceReferences.h"
#include "Renderer/PostProcessing/Fog/FogManager.h"
#include "Editor/EditorWindows/SceneWindow.h"
#include "CameraShake/KickShake.h"
#include "Paths.h"
#include "AssetDatabase.h"
#include "Engine/GameObject/Components/BezierSplineComponent.h"
#include "Engine/GameObject/Components/FolliageCuller.h"

void InitConsole()
{
#ifndef _RETAIL
	AllocConsole();
	freopen("CONIN$", "r", stdin);
	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);

	setbuf(stdin, NULL);
	setbuf(stdout, NULL);
	setbuf(stderr, NULL);
#endif
}

void CloseConsole()
{
#ifndef _RETAIL
	fclose(stdin);
	fclose(stdout);
	fclose(stderr);
#endif
}

// thread_local LinearAllocator Engine::Engine::myAllocator;

Engine::Engine::Engine()
{
}

Engine::Engine::~Engine()
{
	// WARNING:
	// DO NOT CHANGE THE ORDER IN HERE WITHOUT THINKING
	// SOME STUFF MUST BE DESTROYED IN THE SPECIFIC ORDER BELOW

	myGlobalResourceReferences = nullptr;

	// Must manually release the editor FIRST otherwise it crashes
	// if editor holds onto a reference to a gameobject that is kept alive
	// even after the scene its contained within is destroyed causing a 
	// crash when that gameobject tries to access its scene
	if (GetEngine().IsEditorEnabled())
	{
		for (auto& [name, scene] : mySceneManager->myScenes)
		{
			if (&GetEngine().GetEditor() == scene.get())
			{
				mySceneManager->myScenes.erase(name);
				break;
			}
		}
	}
	myEditor = nullptr;

	GraphNodeRegistry::Destroy();

	// This calls the destructor and deletes the memory manually
	myCamera = nullptr;
	mySceneManager = nullptr;
	myGraphicsEngine = nullptr;
	// myCameraFactory = nullptr;
	myTimer = nullptr;
	myInputManager = nullptr;
	myJsonManager = nullptr;
	myFileWatcher = nullptr;
	myTextFactory = nullptr;
	myDebugDrawerSettings = nullptr;

	myEngineResourceRefs = nullptr;

	myResourceManager = nullptr;

	// Close the console after all systems has been destroyed
	// otherwise it will print after the console was destroyed and cause errors
	CloseConsole();

	Log::Destroy();
	AssetDatabase::Destroy();
}

bool Engine::Engine::Init(const EngineSettings& aEngineSettings)
{
	Log::Create();
	AssetDatabase::Create("Assets");

	myPaths = MakeOwned<Paths>();

	myEngineResourceRefs = MakeOwned<ResourceReferences>();

	// Only did it to support physx debug DLLs
#ifdef _DEBUG
	const BOOL result = SetDllDirectory(L"LibrariesDebug");
#else
	const BOOL result = SetDllDirectory(L"Libraries");
#endif

	if (!result)
	{
		printf("SetDllDirectory failed\n");
		return false;
	}

	InitConsole();

	myParallelizationThreadPool = MakeOwned<ThreadPool>(std::max(1u, std::thread::hardware_concurrency()));




	// myParallelizationThreadPool->AddTask(
	// 	[]()
	// 	{
	// 		myAllocator = LinearAllocator(1 * 1000 * 1000 * 50);
	// 	});
	// 
	// myParallelizationThreadPool->WaitForTasks();




	SceneRegistry::RegisterType<FolderScene>("Default");
	SceneRegistry::RegisterSceneTypes();

	myCommandHandler = MakeOwned<CommandHandler>();

	if (!myCommandHandler->Init())
	{
		LOG_ERROR(LogType::Engine) << "CommandHandler failed to init";
		return false;
	}

	myIngameConsole = MakeOwned<IngameConsole>(*myCommandHandler);

	if (!myIngameConsole->Init())
	{
		LOG_ERROR(LogType::Engine) << "IngameConsole failed to init";
		return false;
	}

	WindowData windowData;
	{
		windowData.myTitle
			= std::wstring(aEngineSettings.WindowTitle.begin(), aEngineSettings.WindowTitle.end());

		windowData.myInstanceHandle = GetModuleHandle(NULL);

		windowData.myCursorHandle = aEngineSettings.myCursorHandle;

		windowData.myWindowSize = aEngineSettings.WindowSize;
		windowData.myTargetSize = aEngineSettings.myTargetWindowSize;
		windowData.myWindowState = aEngineSettings.myWindowState;

		// NOTE: Do not edit render size, it is automatically calulcated if zero
		//windowData.myWindowSize = CU::Vector2ui(1264, 681);
		windowData.myWindowFlags = WindowFlags_CalculateRenderingSizeAutomatically;

		windowData.myWindowProc
			= [this](HWND aWindowHandle, UINT aMessage, WPARAM aWparam, LPARAM aLparam)
		{
			return WndProc(aWindowHandle, aMessage, aWparam, aLparam);
		};
	}

	myTimer = MakeOwned<C::Timer>();
	myInputManager = MakeOwned<C::InputManager>();

	if (aEngineSettings.myEnableFilewatcher)
	{
		myFileWatcher = MakeOwned<FileWatcher>();
	}
	else
	{
		myFileWatcher = MakeOwned<FileWatcherNull>();
	}

	myFileWatcher->Start();

	myJsonManager = MakeOwned<JsonManager>();
	myJsonManager->SetFileWatcher(*myFileWatcher);

#ifndef _RETAIL
	if (!LoadLoggingSettings())
	{
		return false;
	}
#endif

	myGraphicsEngine = MakeOwned<GraphicsEngine>(myFileWatcher.get());

	if (!myGraphicsEngine->InitFramework(windowData))
	{
		LOG_ERROR(LogType::Engine) << "GraphicsEngine framework failed to init";
		return false;
	}

	myResourceManager = MakeOwned<ResourceManager>(aEngineSettings.myResourceManagerSettings);
	GResourceManager = myResourceManager.get();

	if (!myResourceManager->Init(*myGraphicsEngine, myFileWatcher.get()))
	{
		LOG_ERROR(LogType::Engine) << "ResourceManager failed to init";
		return false;
	}

	if (!myGraphicsEngine->Init(windowData, *myResourceManager, *myEngineResourceRefs))
	{
		LOG_ERROR(LogType::Engine) << "Graphics engine failed to initialize";

		return false;
	}

	myInputManager->SetWindow(myGraphicsEngine->GetWindowHandler().GetWindowHandle());

	myDebugDrawerSettings = MakeOwned<DebugDrawerSettings>();

#ifndef _RETAIL
	if (!myDebugDrawerSettings->LoadDebugDrawSettings())
	{
		LOG_ERROR(LogType::Engine) << "Failed to load debug draw settings";
		return false;
	}
#endif

	mySceneManager = MakeOwned<SceneManager>();

	myTimeSystem = MakeOwned<TimeSystem>();
	myPhysics = MakeOwned<Physics>();
	if (!myPhysics->Init())
	{
		LOG_ERROR(LogType::Engine) << "Physics failed to initialize";
		return false;
	}

	myCameraShakeManager = MakeOwned<CameraShakeManager>();

	if (!myCameraShakeManager->Init(*myJsonManager))
	{
		LOG_ERROR(LogType::Engine) << "CameraShakeManager failed to initialize";
		return false;
	}

	// TODO: Window size or Rendering size?
	// myCameraFactory = MakeOwned<Renderer::CameraFactory>();
	// myCameraFactory->Start(myGraphicsEngine->GetWindowHandler().GetWindowSize<float>());

	myTextFactory
		= MakeOwned<TextFactory>(myGraphicsEngine->GetDxFramework().GetDevice());

	if (!myTextFactory->Init())
	{
		LOG_ERROR(LogType::Engine) << "Text factory failed to init";
		return false;
	}

	DebugMenu::ToggleDebugMenu(false);

	myGlobalResourceReferences = MakeOwned<ResourceReferences>();

	if (!myGlobalResourceReferences->AddReferencesFromJson("Assets\\Json\\GlobalResources.json"))
	{
		LOG_ERROR(LogType::Resource) << "Failed to load global resources";
		return false;
	}

	myGlobalResourceReferences->RequestAll();

	// TEMPORARY DEBUG SCENE INIT
	// auto scene = std::make_shared<Scene>();
	// mySceneManager->SetScene(scene);

	RegistrateComponents();
	GraphNodeRegistry::PopulateTypes();
	LOG_INFO(LogType::Engine) << "Starting wait for engine initialization!";

	// All should already be requested, but just in case
	myEngineResourceRefs->RequestAll();

	// TODO: Not all engine resources are using the myEngineResourceReferences
	// make them do that

	// Wait for the engine resources to finish loading (quite disgusting, but whatever)
	do
	{
		myResourceManager->TransferResourceToRenderThread();
		// std::this_thread::sleep_for(std::chrono::milliseconds(1));
		Sleep(0);
	} while (!GetEngine().GetResourceManager().IsAllResourcesLoaded() ||
		!myEngineResourceRefs->AreAllFinishedLoading());

	LOG_INFO(LogType::Engine) << "Engine has been initialized!";

	return true;
}

void Engine::Engine::Update()
{
	AssetDatabase::Update();

	myInputManager->BeginFrame();

	if (myGraphicsEngine->GetWindowHandler().IsDestroyed())
	{
		return;
	}

	UpdateViewport();

	ZoneNamedN(engineupdatescope, "Engine::Update", true);

	auto& input = *myInputManager;

	if (input.IsKeyDown(C::KeyCode::F4))
	{
		if (myEditor && !myEditor->IsEnabled())
		{
			myEditor->ReturnToEditor();
		}
	}

#ifndef _RETAIL
	if (input.IsKeyDown(C::KeyCode::Oem5))
	{
		myIngameConsole->SetEnabled(!myIngameConsole->IsEnabled());
	}

	if (input.IsKeyDown(C::KeyCode::F1))
	{
		DebugMenu::ToggleDebugMenu(!DebugMenu::Enabled());
	}

	if (input.IsKeyDown(C::KeyCode::F7))
	{
		Physics::ConnectPVD();
	}
#endif

	myDebugDrawerSettings->Update();

	myGraphicsEngine->Update();

	myGraphicsEngine->BeginFrameImgui();
	myGraphicsEngine->ImGuiNewFrame();

	DebugDrawSystems(Time::DeltaTime);

	UpdateTimings();

	{
		ZoneNamedN(filewatchscope, "FlushChangedFiles", true);
		myFileWatcher->FlushChangedFiles();
	}

	myResourceManager->TransferResourceToRenderThread();

	myTimeSystem->Update(Time::DeltaTime);

	AudioManager::GetInstance()->Update();

	{
		ZoneNamedN(allupdatesscope, "All UpdateSystems", true);

		mySceneManager->UpdateAllScenes(myTimeScale);
	}

	{
		// TODO: Move the scene updates and stuff into here
		// but, problem is when we do, it starts loading more stuff, causing it to for 1 frame
		// come in here and render, then the next start loading more

		// Rendering
		{
			ZoneNamedN(renderscope, "Rendering", true);

			START_TIMER(rendering);

			myGraphicsEngine->BeginFrame();

			if (mySceneManager->GetMainScene() != nullptr)
			{
				// mySceneManager->RenderReal(*myGraphicsEngine);

				// We render the main scene to the backbuffer
				// internally in that scene, it can also render scenes to its
				// internal texture, that can be used to render another scene within
				// that scene to e.g. a imgui window
				myGraphicsEngine->GetRenderManager().RenderSceneToBackBuffer(
					*mySceneManager->GetMainScene());
			}

			DebugMenu::RenderImgui();

			if (myIngameConsole->IsEnabled())
			{
				myIngameConsole->DrawImgui();
			}

			// An issues here is that, if we're having the editor scene loaded, then loading another scene
			// and setting that new scene as active, it will still draw editor imgui still
			// because of the imgui call below, the imgui is currently not draw per scene texture
			myGraphicsEngine->ImGuiRenderFrame();

			myGraphicsEngine->GetRenderManager().RenderResultToBackBuffer(
				*mySceneManager->GetMainScene());

			myGraphicsEngine->EndFrame();

			myPrevRenderingFrameMs = END_TIMER_GET_RESULT_MS(rendering);
		}
	}

	// Must be at end?
	myInputManager->Update();

	FrameMark;
}

LRESULT CALLBACK Engine::Engine::WndProc(
	HWND aWindowHandle, UINT aMessage, WPARAM aWparam, LPARAM aLparam)
{
	if (myInputManager->UpdateEvents(aWindowHandle, aMessage, aWparam, aLparam))
	{
		return 1;
	}

	// TODO: return value is currently not handled on the receiving end in the renderer
	return 0;
}

Engine::Engine& Engine::Engine::GetInstance()
{
	assert(myInstance != nullptr);
	return *myInstance;
}

bool Engine::Engine::LoadLoggingSettings()
{
	const std::string path = "Dev/LoggingSettings.json";
	std::ifstream file(path);

	if (!file.is_open())
	{
		std::cout << "Failed to open " << path << std::endl;
		return false;
	}

	const bool ignoreComments = true;
	const bool allowExceptions = false;
	nlohmann::json j = nlohmann::json::parse(file, nullptr, allowExceptions, ignoreComments);

	// If failed to parse json file
	if (j.is_discarded())
	{
		std::cout << "Failed to parse " << path << std::endl;
		return false;
	}

	Log::SetIsLoggingEnabled(j["LoggingEnabled"]);

	for (int i = 0; i < static_cast<int>(LogType::Count); ++i)
	{
		const auto logChannelEnum = static_cast<LogType>(i);
		if (j["LogChannels"].contains(LogChannelEnumToString(logChannelEnum)))
		{
			Log::SetChannelState(
				logChannelEnum, j["LogChannels"][LogChannelEnumToString(logChannelEnum)]);
		}
	}

	return true;
}

bool Engine::Engine::SaveLoggingSettings()
{
	nlohmann::json j;

	j["LoggingEnabled"] = Log::GetIsLoggingEnabled();
	j["LogChannels"] = nlohmann::json::object();

	for (int i = 0; i < static_cast<int>(LogType::Count); ++i)
	{
		const auto logChannelEnum = static_cast<LogType>(i);

		j["LogChannels"][LogChannelEnumToString(logChannelEnum)]
			= Log::GetChannelState(logChannelEnum);
	}

	const std::string path = "Dev/LoggingSettings.json";

	std::ofstream file(path);

	file << std::setw(4) << j;

	file.close();

	return true;
}

void Engine::Engine::DrawLogDebugMenu()
{
	DebugMenu::AddMenuEntry("Log",
		[this]()
		{
			bool isLoggingEnabled = Log::GetIsLoggingEnabled();
			if (ImGui::Checkbox("Logging Enabled", &isLoggingEnabled))
			{
				Log::SetIsLoggingEnabled(isLoggingEnabled);

				SaveLoggingSettings();
			}

			if (ImGui::SmallButton("Select all"))
			{
				for (int i = 0; i < static_cast<int>(LogType::Count); ++i)
				{
					Log::SetChannelState(static_cast<LogType>(i), true);
				}

				SaveLoggingSettings();
			}

			ImGui::SameLine();

			if (ImGui::SmallButton("Deselect all"))
			{
				for (int i = 0; i < static_cast<int>(LogType::Count); ++i)
				{
					Log::SetChannelState(static_cast<LogType>(i), false);
				}

				SaveLoggingSettings();
			}

			static ImGuiTableFlags flags = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg
				| ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable
				| ImGuiTableFlags_Hideable;

			bool clickedEditTexture = false;

			if (ImGui::BeginTable("logchannels", 1, flags))
			{
				ImGui::TableSetupColumn("ChannelName", ImGuiTableColumnFlags_WidthStretch);
				ImGui::TableHeadersRow();

				for (int i = 0; i < static_cast<int>(LogType::Count); ++i)
				{
					const auto channelEnum = static_cast<LogType>(i);

					ImGui::PushID(i);

					ImGui::TableNextRow();

					for (int column = 0; column < 1; column++)
					{
						ImGui::TableSetColumnIndex(column);

						switch (column)
						{
						case 0:
						{
							bool isChannelEnabled = Log::GetChannelState(channelEnum);
							if (ImGui::Checkbox(LogChannelEnumToString(channelEnum).c_str(),
								&isChannelEnabled))
							{
								Log::SetChannelState(channelEnum, isChannelEnabled);

								SaveLoggingSettings();
							}
						}
						break;
						default:
							assert(false);
							break;
						}
					}

					ImGui::PopID();
				}
				ImGui::EndTable();
			}
		});
}

void Engine::Engine::DrawRenderingStatsDebugMenu(const TimeStamp aTs)
{
	// Gather rendering statistics
	DebugMenu::AddMenuEntry("Rendering Statistics",
		[&]()
		{
			const auto prevFrameTimeMs = aTs.GetMilliseconds();

			ImGui::Text("Prev Frame Time: %.3f ms", prevFrameTimeMs);
			ImGui::Text("All Rendering Time: %.3f ms", myPrevRenderingFrameMs);

			// const auto& renderingStats = myGraphicsEngine->GetRenderingStats();

			if (ImGui::CollapsingHeader("All Scenes"))
			{
				RenderingStatistics total;

				for (const auto& [handle, scene] : GetEngine().GetSceneManager().GetScenes())
				{
					total = total + scene->GetRendererScene().GetStats();
				}

				DrawSceneStats(total);
			}

			for (const auto& [name, scene] : GetEngine().GetSceneManager().GetScenes())
			{
				if (ImGui::CollapsingHeader(scene->GetName().c_str()))
				{
					const auto& sceneStats = scene->GetRendererScene().GetStats();

					DrawSceneStats(sceneStats);
				}
			}
		});
}

void Engine::Engine::DrawSceneStats(const RenderingStatistics& aStats) const
{
	const auto drawRenderTimeText = [this](const std::string aName, const float aMs)
	{
		ImGui::Text("%s: %.3f ms (%.1f%% of whole frame)",
			aName.c_str(),
			aMs,
			aMs / myPrevRenderingFrameMs * 100.f);
	};

	const auto& deferredStats = aStats.myDeferredRenderingStats;
	const auto& gbufferStats = aStats.myGBufferRenderingStats;
	const auto& forwardStats = aStats.myForwardRenderingStats;
	const auto& textStats = aStats.myTextRenderingStats;
	const auto& environmentLightShadowStats = aStats.myEnvironmentLightShadowRenderStats;
	const auto& spotLightShadowStats = aStats.mySpotLightShadowRenderStats;
	const auto& pointLightShadowStats = aStats.myPointLightShadowRenderStats;

	ImGui::Text("Total DrawCalls: %d",
		gbufferStats.myMeshDrawCalls + deferredStats.myEnvironmentLightDrawCalls
		+ deferredStats.myPointLightDrawCalls + deferredStats.mySpotLightDrawCalls
		+ forwardStats.myMeshDrawCalls + textStats.myDrawCalls
		+ environmentLightShadowStats.myDrawCalls + spotLightShadowStats.myDrawCalls
		+ pointLightShadowStats.myDrawCalls);

	if (ImGui::TreeNode("Shadows"))
	{
		const int totalShadowDrawCalls = environmentLightShadowStats.myDrawCalls
			+ spotLightShadowStats.myDrawCalls + pointLightShadowStats.myDrawCalls;

		const float totalShadowRenderTime = environmentLightShadowStats.myRenderTime
			+ spotLightShadowStats.myRenderTime + pointLightShadowStats.myRenderTime;

		ImGui::Text("Total DrawCalls: %d", totalShadowDrawCalls);
		drawRenderTimeText("Total RenderTime", totalShadowRenderTime);

		if (ImGui::TreeNode("Environment Light"))
		{
			ImGui::Text("DrawCalls: %d", environmentLightShadowStats.myDrawCalls);
			drawRenderTimeText("RenderTime", environmentLightShadowStats.myRenderTime);
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Spot Light"))
		{
			ImGui::Text("DrawCalls: %d", spotLightShadowStats.myDrawCalls);
			drawRenderTimeText("RenderTime", spotLightShadowStats.myRenderTime);
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Point Light"))
		{
			ImGui::Text("DrawCalls: %d", pointLightShadowStats.myDrawCalls);
			drawRenderTimeText("RenderTime", pointLightShadowStats.myRenderTime);
			ImGui::TreePop();
		}

		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Deferred Rendering"))
	{
		ImGui::Text("GBufferDrawCalls: %d", gbufferStats.myMeshDrawCalls);
		drawRenderTimeText("GBufferRenderTime", gbufferStats.myRenderTime);

		ImGui::Text("EnvironmentLightDrawCalls: %d", deferredStats.myEnvironmentLightDrawCalls);
		ImGui::Text("SpotLightDrawCalls: %d", deferredStats.mySpotLightDrawCalls);
		ImGui::Text("PointLightDrawCalls: %d", deferredStats.myPointLightDrawCalls);
		drawRenderTimeText("RenderTime", deferredStats.myRenderTime);
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Forward Rendering"))
	{
		ImGui::Text("MeshDrawCalls: %d", forwardStats.myMeshDrawCalls);
		drawRenderTimeText("RenderTime", forwardStats.myRenderTime);
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Text Rendering"))
	{
		ImGui::Text("DrawCalls: %d", textStats.myDrawCalls);
		drawRenderTimeText("RenderTime", forwardStats.myRenderTime);
		ImGui::TreePop();
	}
}

void Engine::Engine::DebugDrawSystems(const float aDeltaTime)
{
	ZoneScopedN("DebugDrawSystems");

	DrawLogDebugMenu();

	DrawTestingDebugMenu();

	//if (mySceneManager->GetScene())
	//{
	//	mySceneManager->GetScene()->DrawDebugMenu();
	//}

	myGraphicsEngine->DrawDebugMenu();

	myDebugDrawerSettings->DrawDebugMenu();

	DrawRenderingStatsDebugMenu(aDeltaTime);

	DrawEngineDebugMenu();
}

void Engine::Engine::DrawTestingDebugMenu()
{
	DebugMenu::AddMenuEntry("Testing",
		[&]()
		{
			//ImGui::Text("Camera Shake");

			//static std::pair<std::string, ShakeType> selectedShakeName;

			//if (ImGui::BeginCombo("Shakes", selectedShakeName.first.c_str()))
			//{
			//	const auto shakes = myCameraShakeManager->GetShakes();

			//	for (const auto& [name, shakeType] : shakes)
			//	{
			//		if (ImGui::Selectable(name.c_str(), selectedShakeName.first == name))
			//		{
			//			selectedShakeName.first = name;
			//			selectedShakeName.second = shakeType;
			//		}
			//	}

			//	ImGui::EndCombo();
			//}

			//if (ImGui::Button("Play Shake"))
			//{
			//	if (!selectedShakeName.first.empty())
			//	{
			//		auto mainCameraGo = mySceneManager->GetMainScene()->GetMainCameraGameObject();
			//		auto shakeComponent = mainCameraGo->GetComponent<CameraShakeComponent>();

			//		switch (selectedShakeName.second)
			//		{
			//		case ShakeType::PerlinShake:
			//			shakeComponent->AddPerlinShake(
			//				myCameraShakeManager->GetPerlinShake(selectedShakeName.first));
			//			break;
			//		default:
			//			break;
			//		}
			//	}
			//}

			//if (ImGui::Button("test kick"))
			//{
			//	auto mainCameraGo = mySceneManager->GetMainScene()->GetMainCameraGameObject();
			//	auto shakeComponent = mainCameraGo->GetComponent<CameraShakeComponent>();

			//	KickShakeDesc desc;
			//	Displacement dir(Vec3f(0.f, -100.f, 0.f), Vec3f(0.f, 0.1f, 0.f));

			//	shakeComponent->AddKickShake(desc, dir);
			//}

			ImGui::Separator();
		});
}

void Engine::Engine::UpdateViewport()
{
	if (IsEditorEnabled() &&
		&myEditor->GetSceneWindow() != nullptr &&
		myEditor->GetEditingScene() != nullptr)
	{
		// Calculate the viewport from the scenewindow within the editor
		const auto& pos = myEditor->GetSceneWindow().GetWindowPosition();
		const auto& size = myEditor->GetSceneWindow().GetWindowSize();

		myViewport.myPosition = pos;
		myViewport.myRendererSize = size;

		Vec2f mousePos = myInputManager->GetMousePosition().CastTo<float>();
		mousePos -= pos;

		const auto windowSize = GetEngine().GetWindowSize();

		const float ratioX = windowSize.x / size.x;
		const float ratioY = windowSize.y / size.y;

		mousePos.x *= ratioX;
		mousePos.y *= ratioY;

		myViewport.myRelativeMousePosition = mousePos;

		mousePos.x /= GetEngine().GetWindowSize().x;
		mousePos.y /= GetEngine().GetWindowSize().y;

		myViewport.myRelativeNormalizedMousePosition = mousePos;
	}
	else
	{
		// If no editor, we assume the game is the whole window
		const Vec2f windowPos = myGraphicsEngine->GetWindowHandler().GetPosition().CastTo<float>();
		const Vec2f renderingSize = myGraphicsEngine->GetWindowHandler().GetRenderingSize().CastTo<float>();

		myViewport.myPosition = windowPos;
		myViewport.myRendererSize = renderingSize;

		Vec2f mousePos = myInputManager->GetMousePosition().CastTo<float>();

		const auto windowSize = GetEngine().GetWindowSize();

		const float ratioX = windowSize.x / renderingSize.x;
		const float ratioY = windowSize.y / renderingSize.y;

		mousePos.x *= ratioX;
		mousePos.y *= ratioY;

		myViewport.myRelativeMousePosition = mousePos;

		mousePos.x /= GetEngine().GetWindowSize().x;
		mousePos.y /= GetEngine().GetWindowSize().y;

		myViewport.myRelativeNormalizedMousePosition = mousePos;
	}

	//for (const auto& scene : mySceneManager->GetScenes())
	//{
	//	// Find the first enabled game scene and use that to get position
	//	// and size of its viewport
	//	if (!scene.second->IsEditorScene() &&
	//		scene.second.get() != static_cast<Scene*>(&GetEngine().GetEditor()) &&
	//		scene.second->IsEnabled())
	//	{
	//		const auto& gameScene = *scene.second;

	//		const auto& sceneWindowSize = gameScene.GetWindowSize();
	//		const auto& sceneWindowPos = gameScene.GetWindowPos();

	//		const int x = static_cast<int>(sceneWindowPos.x + (sceneWindowSize.x) / 2.f);
	//		const int y = static_cast<int>(sceneWindowPos.y + (sceneWindowSize.y) / 2.f);

	//		SetCursorPos(myPosition.x + x, myPosition.y + y);

	//		break;
	//	}
	//}
}

void Engine::Engine::DrawEngineDebugMenu()
{
	DebugMenu::AddMenuEntry("Engine",
		[&]()
		{
			ImGui::Text("Viewport Position: (%f, %f)", myViewport.myPosition.x, myViewport.myPosition.y);
			ImGui::Text("Viewport Renderer Size: (%f, %f)", myViewport.myRendererSize.x, myViewport.myRendererSize.y);
			ImGui::Text("Viewport Rel Mouse Pos: (%f, %f)", myViewport.myRelativeMousePosition.x, myViewport.myRelativeMousePosition.y);
			ImGui::Text("Viewport Rel Mouse Pos Norm: (%f, %f)", myViewport.myRelativeNormalizedMousePosition.x, myViewport.myRelativeNormalizedMousePosition.y);
		});
}

void Engine::Engine::UpdateTimings()
{
	myTimer->Update();

	// if the timer goes above 1 seconds, we can assume we had hit a breakpoint
	// as a solution we simply manually set the delta time to avoid a big "jump"
	// in time after we resume
	if (myTimer->GetDeltaTime() > 1.f)
	{
		myTimer->SetDeltaTime(1.f / 60.f);

		LOG_WARNING(LogType::Engine)
			<< "Artifially set the deltatime to 1 / 60 due to DeltaTime going "
			"above 1 second";
	}


	float halfTimeScale = (1.0f - Time::TimeScale * Time::PauseTimeScale) * 0.5f;

	float halfScale = Time::TimeScale * Time::PauseTimeScale + halfTimeScale;
	float extraScale = Time::TimeScale * Time::PauseTimeScale - (halfTimeScale * 0.5f);

	Time::DeltaTimeUnscaled = myTimer->GetDeltaTime();
	Time::DeltaTime = myTimer->GetDeltaTime() * Time::TimeScale * Time::PauseTimeScale;
	Time::DeltaTimeHalfScaled = myTimer->GetDeltaTime() * halfScale;
	Time::DeltaTimeExtraScaled = myTimer->GetDeltaTime() * extraScale;

	Time::TotalTime = myTimer->GetTotalTime();


	{
		//LOG_INFO(LogType::Game) << "Half is  " << halfScale << "	 Extra is " << extraScale;
	}
}

Engine::CameraFactory& Engine::Engine::GetCameraFactory()
{
	return myGraphicsEngine->GetCameraFactory();
}

Engine::CameraShakeManager& Engine::Engine::GetCameraShakeManager()
{
	return *myCameraShakeManager;
}

Engine::SceneManager& Engine::Engine::GetSceneManager()
{
	return *mySceneManager;
}

Physics& Engine::Engine::GetPhysics()
{
	return *myPhysics;
}

Engine::TimeSystem& Engine::Engine::GetTimeSystem()
{
	return *myTimeSystem;
}

Engine::TextFactory& Engine::Engine::GetTextFactory()
{
	return *myTextFactory;
}

Engine::GraphicsEngine& Engine::Engine::GetGraphicsEngine()
{
	return *myGraphicsEngine;
}

ThreadPool& Engine::Engine::GetParallelizationThreadPool()
{
	return *myParallelizationThreadPool;
}

Engine::DebugDrawerSettings& Engine::Engine::GetDebugDrawerSettings()
{
	return *myDebugDrawerSettings;
}

FileWatcher& Engine::Engine::GetFileWatcher()
{
	return *myFileWatcher;
}

JsonManager& Engine::Engine::GetJsonManager()
{
	return *myJsonManager;
}

void Engine::Engine::StartEditor()
{
	myEditor = MakeShared<Editor>();

	myEditorSceneHandle = mySceneManager->LoadSceneAsync(myEditor);
}

bool Engine::Engine::IsEditorEnabled() const
{
	return myEditor && myEditor->IsEnabled();
}

Engine::ResourceManager& Engine::Engine::GetResourceManager()
{
	return *myResourceManager;
}

Engine::CommandHandler& Engine::Engine::GetCommandHandler()
{
	return *myCommandHandler;
}

Engine::Editor& Engine::Engine::GetEditor()
{
	return *myEditor;
}

Engine::SceneHandle Engine::Engine::GetEditorSceneHandle() const
{
	return myEditorSceneHandle;
}

Engine::ResourceReferences& Engine::Engine::GetEngineResourceRefs()
{
	return *myEngineResourceRefs;
}

Vec2f Engine::Engine::GetWindowSize() const
{
	return myGraphicsEngine->GetWindowHandler().GetWindowSize<float>();
}

Vec2f Engine::Engine::GetRenderingSize() const
{
	return myGraphicsEngine->GetWindowHandler().GetRenderingSize<float>();
}

const Vec2ui& Engine::Engine::GetTargetSize() const
{
	return myGraphicsEngine->GetWindowHandler().GetTargetSize();
}

const Engine::Viewport& Engine::Engine::GetViewport() const
{
	return myViewport;
}

void Engine::Engine::SetCursorMode(const CursorMode aCursorMode)
{
	myGraphicsEngine->GetWindowHandler().SetCursorMode(aCursorMode);
}

Engine::CursorMode Engine::Engine::GetCursorMode() const
{
	return myGraphicsEngine->GetWindowHandler().GetCursorMode();
}

void Engine::Engine::SetCursorVisiblity(const bool aCursorVisible)
{
	myGraphicsEngine->GetWindowHandler().SetCursorVisibility(aCursorVisible);
}

bool Engine::Engine::IsCursorVisible() const
{
	return myGraphicsEngine->GetWindowHandler().IsCursorVisible();
}

const Engine::Paths& Engine::Engine::GetPaths() const
{
	return *myPaths;
}

void Engine::Engine::SetInstance(Engine& aEngine)
{
	myInstance = &aEngine;
}

C::Timer& Engine::Engine::GetTimer()
{
	return *myTimer;
}

void Engine::Engine::SetTimeScale(float aTimeScale)
{
	myTimeScale = aTimeScale;
}

float Engine::Engine::GetTimeScale() const
{
	return myTimeScale;
}

C::InputManager& Engine::Engine::GetInputManager()
{
	return *myInputManager;
}

Shared<Engine::Scene> Engine::GetScene()
{
	return GetEngine().GetSceneManager().GetMainScene();
}