#pragma once

#define _CRT_SECURE_NO_WARNINGS

#define NOMINMAX
#define _WINSOCKAPI_

#include "../../../tracy-0.7.8/Tracy.hpp"

#include <d3d11.h>
#include "../../../tracy-0.7.8/TracyD3D11.hpp"

#include <assert.h>
#include <fstream>
#include <stdint.h>
#include <array>
#include <vector>
#include <chrono>
#include <functional>
#include <string>
#include <Windows.h>
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <variant>
#include <any>
#include <queue>
#include <d3dcompiler.h>
#include <d3d11shader.h>
#include <algorithm>
#include <execution>
#include <filesystem>
#include <system_error>
#include <stack>
#include <Shlobj.h>
#include <random>
#include <set>
#include <optional>
#include <future>

// CommonUtilities
#include <Common/Math/Math.h>
#include <Common/Timer.h>
#include <Common/InputManager.h>
#include <Common/Sphere.hpp>
#include <Common/PlaneVolume.hpp>
#include <Common/heap.hpp>
#include <Common/RingBuffer.h>
#include "Common/ThreadPool/ThreadPool.h"
#include <Common/LinearAllocator.h>
#include <Common/BetterEnums/enum.h>
#include "Common/delaunay.h"

// rapidjson
#include "rapidjson/document.h"

// DirectXTK
#include <DirectXTK/DDSTextureLoader.h>

#include <flatbuffers/flatbuffers.h>

// Common
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include "imgui_stdlib.h"
#include "ImGuizmo.h"
#include <imgui_node_editor.h>
#include "Common/Types.h"
#include "Common/Log.h"
#include "Common/Random.h"
#include "Common/TimerTemp.h"
#include "Common/FileWatcher.h"
#include "Common/nlohmann/json.hpp"
#include "Common/FileIO.h"
#include "Common/JsonManager/JsonManager.h"
#include "Common/Path.h"
#include "Common/BinaryWriter.h"
#include "Common/BinaryReader.h"
#include "Common/Math/Math.h"
#include "Common/Math/PerlinNoise.h"
#include "Common/ObjectPoolGrowable.h"
#include "Common/ObjectPool.h"
#include "Common/ObjectPoolUntyped.h"
#include "Common\StringUtilities.h"
#include "Common/VectorOnStack.h"

#include "Engine/Time.h"

// PhysX
#include <foundation/PxSimpleTypes.h>
#include <foundation/PxVec3.h>
#include <PxPhysics.h>
#include <PxRigidDynamic.h>
#include <PxPhysicsAPI.h>

namespace C = Common;

template <typename T>
using Vec2 = Common::Vector2<T>;
template <typename T>
using Vec3 = Common::Vector3<T>;
template <typename T>
using Vec4 = Common::Vector4<T>;
using Vec2ui = Common::Vector2ui;
using Vec2f = Common::Vector2f;
using Vec3f = Common::Vector3f;
using Vec4f = Common::Vector4f;
using Quatf = Common::Quaternion;
using Mat4f = Common::Matrix4f;
using Mat3f = Common::Matrix3f;

/*
	ComPtr Best Pratices:
	Reference: https://github.com/microsoft/DirectXTK/wiki/ComPtr

	* When taking array of pointers user .GetAddressOf()

	* When building array of d3d interface pointers, build one as local variable
		- ID3D11SamplerState* samplers[] = { sampler1.Get(), sampler2.Get() };

	* If created fresh ComPtr and using a factory, can use: .GetAddressOf()
		-	hr = D3D11CreateDevice( ..., device.GetAddressOf(), ... );
			or
			hr = D3D11CreateDevice( ..., &device, ... );

	* Passing ComPtr as by value increment/decrements the reference count
		Avoid by passing by const& or raw pointer. Raw pointers are more flexible.

	* Can use .Reset() to release reference.

	* Check for null: by if (!SomeComPtrHere) { it is nullptr }


*/
#include <wrl/client.h>
using namespace Microsoft::WRL;

// Assimp
#include <assimp/mesh.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include "assimp\quaternion.h"
#include "assimp\vector3.h"
#include "assimp\matrix4x4.h"
#include "assimp\anim.h"

// libdeflate
#include <libdeflate/libdeflate.h>

// engine
#include "Engine/Shortcuts.h"


#ifdef _DEBUG
#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
// Replace _NORMAL_BLOCK with _CLIENT_BLOCK if you want the
// allocations to be of _CLIENT_BLOCK type
#else
#define DBG_NEW new
#endif