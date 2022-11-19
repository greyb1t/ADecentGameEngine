#pragma once

#define NOMINMAX
#define _WINSOCKAPI_

#include "../../../tracy-0.7.8/Tracy.hpp"

//#undef NOMINMAX

#include <Windows.h>
#include <iostream>
#include <any>
#include <fstream>
#include <vector>
#include <string>
#include <queue>
#include <variant>
#include <set>
#include <d3d11.h>
#include <numeric>

#include "Common/Types.h"

#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include "imgui_stdlib.h"
#include "Common/JsonManager/JsonManager.h"
#include "Common\TimerTemp.h"
#include "Common/Math/Math.h"
#include "Common/StringUtilities.h"

#include <Common/nlohmann/json.hpp>

#include "Common/Log.h"
#include "Common/Random.h"
#include "Common/AnimationCurve.h"
#include "Common/Path.h"
#include "Common/ObjectPool.h"
#include "Common/Math/Math.h"
#include "Common/InputManager.h"
#include "Common/Timer.h"
#include "Common\Heap.hpp"
#include "Common\VectorOnStack.h"
#include "Common/BetterEnums/enum.h"
#include "Common/Math/BezierSpline.h"

//#include "Engine/GameObject/GameObject.h"

#include "Engine/Time.h"
#include "Engine/Shortcuts.h"



#include "Game/MainSingleton/MainSingleton.h"
#include "GameManager/GameManager.h"

#include "Engine/ResourceManagement/ResourceRef.h"
#include "Engine/ResourceManagement/Resources/GameObjectPrefabResource.h"

// PhysX
#include <PxPhysicsAPI.h>

#include "assimp/scene.h"

#include <wrl/client.h>
using namespace Microsoft::WRL;

using namespace nlohmann;

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

#include "Engine/DebugManager/DebugDrawer.h"

#ifdef _DEBUG
#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
// Replace _NORMAL_BLOCK with _CLIENT_BLOCK if you want the
// allocations to be of _CLIENT_BLOCK type
#else
#define DBG_NEW new
#endif