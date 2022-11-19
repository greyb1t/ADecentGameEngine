// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

#include "Common/Math/Math.h"
#include "Common/JsonManager/JsonManager.h"
#include "Common/Types.h"

#include "Common/Log.h"
#include "Common/Path.h"

// because we include Engine::Scene and it includes PhysicsScene.........
#include <foundation/Px.h>
#include <geometry/PxGeometry.h>
#include <PxActor.h>
#include <PxRigidStatic.h>
#include <pvd/PxPvdTransport.h>
#include <PxFoundation.h>
#include <extensions/PxDefaultAllocator.h>
#include <extensions/PxDefaultCpuDispatcher.h>
#include <extensions/PxDefaultErrorCallback.h>
#include <PxPhysicsAPI.h>
#include <d3d11.h>

#include <queue>
#include <Windows.h>
#include <d3d11.h>
#include <any>
#include <variant>

#include <wrl/client.h>
using namespace Microsoft::WRL;

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

#endif //PCH_H

#ifdef _DEBUG
#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
// Replace _NORMAL_BLOCK with _CLIENT_BLOCK if you want the
// allocations to be of _CLIENT_BLOCK type
#else
#define DBG_NEW new
#endif
