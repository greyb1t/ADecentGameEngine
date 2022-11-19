#define NOMINMAX

#include <Windows.h>

#include "Common/Types.h"
#include "Game/Game.h"

#pragma comment(lib, "d3d11.lib")
#ifdef _DEBUG
#pragma comment(lib, "DirectXTK-debug.lib")
#pragma comment(lib, "assimp-vc142-mt.lib")
#pragma comment(lib, "imgui_Debug.lib")
#pragma comment(lib, "imgui_node_editor_Debug.lib")
#pragma comment(lib, "ImGuizmo_Debug.lib")
#else
#pragma comment(lib, "assimp-vc142-mt.lib")
#pragma comment(lib, "DirectXTK-release.lib")
#pragma comment(lib, "imgui_Release.lib")
#pragma comment(lib, "imgui_node_editor_Release.lib")
#pragma comment(lib, "ImGuizmo_Release.lib")
#endif

#include "../tracy-0.7.8/Tracy.hpp"

#ifdef TRACY_ENABLE
void* operator new(std::size_t count)
{
	auto ptr = malloc(count);
	TracyAlloc(ptr, count);
	return ptr;
}

void operator delete(void* ptr) noexcept
{
	TracyFree(ptr);
	free(ptr);
}
#endif

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#include <string>
#include <vector>

struct m
{
	size_t size;
	void* mem;
	std::string file;
	int line;
};

std::vector<m> memory;

void* newerNew(size_t size, std::string file, int line)
{
	void* mem = malloc(size);
	memory.push_back({ size, mem, file, line });
	return mem;
}

void newerDelete(void* mem)
{
	for (int i = 0; i < memory.size(); ++i)
	{
		if (memory[i].mem == mem)
		{
			memory.erase(memory.begin() + i);
			break;
		}
	}
	free(mem);
}

//#define NEW(size) newerNew(size, __FILE__, __LINE__)
//#define DELETE(mem) newerDelete(mem)

int WINAPI wWinMain(
	HINSTANCE aInstanceHandle,
	HINSTANCE aPrevInstance,
	PWSTR aCmdLine,
	int aCmdShow)
{
	aInstanceHandle;
	aPrevInstance;
	aCmdLine;
	aCmdShow;
	{
		Game game;

		if (!game.Init())
		{
			printf("Game failed to init\n");
			Sleep(10000);
			return 1;
		}

		bool shouldRun = true;

		MSG message = { };

		while (shouldRun)
		{
			ZoneScopedN("MessageLoop");

			while (PeekMessage(&message, nullptr, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&message);
				DispatchMessage(&message);

				if (message.message == WM_QUIT)
				{
					shouldRun = false;
				}
			}

			game.Update();
		}
	}

#ifdef DEBUG_MEMORY
	_CrtDumpMemoryLeaks();
#endif
	return 0;
}