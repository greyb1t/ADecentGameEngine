// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

#include "Common/Math/Math.h"

#include <foundation/PxSimpleTypes.h>
#include <foundation/PxVec3.h>
#include <foundation/Px.h>
#include <PxPhysics.h>
#include <PxRigidDynamic.h>
#include <PxPhysicsAPI.h>
#include <geometry/PxGeometry.h>
#include <unordered_map>
#include <vector>
#include <PxFoundation.h>
#include <PxScene.h>
#include <extensions/PxDefaultAllocator.h>
#include <extensions/PxDefaultCpuDispatcher.h>
#include <extensions/PxDefaultErrorCallback.h>
#include <pvd/PxPvdTransport.h>
#include <cassert>
#include <PxMaterial.h>
#include <PxPhysicsVersion.h>
#include <cooking/PxCooking.h>
#include <extensions/PxDefaultStreams.h>
#include <foundation/PxIO.h>
#include <PxSimulationEventCallback.h>
#include <iostream>
#include <PxRigidActor.h>
#include <string>
#include <PxFiltering.h>
#include <PxQueryFiltering.h>
#include <PxShape.h>
#include <PxQueryReport.h>
#include <extensions/PxRigidBodyExt.h>
#include <PxRigidStatic.h>
#include <PxActor.h>
#include <cstdint>
#include "characterkinematic\PxController.h"
#include "characterkinematic\PxCapsuleController.h"

#endif //PCH_H

#ifdef _DEBUG
#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
// Replace _NORMAL_BLOCK with _CLIENT_BLOCK if you want the
// allocations to be of _CLIENT_BLOCK type
#else
#define DBG_NEW new
#endif
