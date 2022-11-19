#include "pch.h"
#include "PhysicsDebugDrawer.h"

#include "Engine/Engine.h"
#include "Engine/DebugManager/DebugDrawer.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/SceneManager.h"

void PhysicsDebugDrawer::DebugDrawShape(const Shape& aShape, const CU::Vector3f& aPosition, const CU::Quaternion& aRotation, const CU::Vector4f& aColor)
{
	if (!GetEngine().GetDebugDrawerSettings().IsDebugFlagActive(DebugDrawFlags::Physics))
	{
		return;
	}

	auto pos = aPosition + aShape.GetLocalPosition() * aRotation;
	switch (aShape.GetType())
	{
	case Shape::eGeometry::eBOX:
	{
		GetEngine()
			.GetSceneManager()
			.GetSceneBeingUpdatedNow()
			->GetRendererScene()
			.GetDebugDrawer()
			.DrawCube3D(DebugDrawFlags::Physics,	
				pos,
				aRotation.EulerAngles(),
				aShape.GetBoxHalfSize(),
				0,
				aColor);
	}
		break;
	case Shape::eGeometry::eSPHERE:
	{
		GetEngine()
			.GetSceneManager()
			.GetSceneBeingUpdatedNow()
			->GetRendererScene()
			.GetDebugDrawer()
			.DrawSphere3D(DebugDrawFlags::Physics, pos, aShape.GetRadius(), 0, aColor);
	}
		break;
	case Shape::eGeometry::eCAPSULE:
	{
		const auto radius = aShape.GetRadius();
		const auto height = aShape.GetHeight();

		GetEngine()
			.GetSceneManager()
			.GetSceneBeingUpdatedNow()
			->GetRendererScene()
			.GetDebugDrawer()
			.DrawLine3D(DebugDrawFlags::Physics,
				pos + CU::Vector3f {0, height * .5f, 0},
				pos - CU::Vector3f {0, height * .5f, 0},
				0,
				aColor);
		GetEngine()
			.GetSceneManager()
			.GetSceneBeingUpdatedNow()
			->GetRendererScene()
			.GetDebugDrawer()
			.DrawSphere3D(DebugDrawFlags::Physics,
				pos + CU::Vector3f {0, height * .5f, 0},
				radius,
				0,
				aColor);
		GetEngine()
			.GetSceneManager()
			.GetSceneBeingUpdatedNow()
			->GetRendererScene()
			.GetDebugDrawer()
			.DrawSphere3D(DebugDrawFlags::Physics,
				pos - CU::Vector3f {0, height * .5f, 0},
				radius,
				0,
				aColor);
	}
		break;
	case Shape::eGeometry::eCONVEX:
	{
		auto vertices = aShape.GetConvexVertices();
		for (int i = 0; i < vertices.size(); i++)
		{
				auto posA = vertices[((i - 1)) % vertices.size()];
				auto posB = vertices[(i) % vertices.size()];
				GetEngine()
					.GetSceneManager()
					.GetSceneBeingUpdatedNow()
					->GetRendererScene()
					.GetDebugDrawer()
					.DrawLine3D(DebugDrawFlags::Physics,
						pos + posA,
						pos + posB,
						0,
						aColor);
		}
	}
	break;
	}
	
	
	/*
	Shape::eGeometry geometry = aShape.

	switch (geoType)
	{
	case physx::PxGeometryType::eSPHERE:
	{
		CU::Vector3f pos{ pxTarget.p.x, pxTarget.p.y, pxTarget.p.z };
		physx::PxSphereGeometry geometry;
		bool isValid = s->getSphereGeometry(geometry);
		if (isValid)
		{
			GetEngine().GetDebugDrawer().DrawSphere3D(pos, geometry.radius, 0, color);
		}
	}
	break;
	case physx::PxGeometryType::ePLANE: break;
	case physx::PxGeometryType::eCAPSULE: break;
	case physx::PxGeometryType::eBOX:
	{
		CU::Vector3f pos{ pxTarget.p.x, pxTarget.p.y, pxTarget.p.z };
		CU::Quaternion rot{ pxTarget.q.w, { pxTarget.q.x, pxTarget.q.y, pxTarget.q.z } };
		physx::PxBoxGeometry box;
		bool isValid = s->getBoxGeometry(box);
		if (isValid)
		{
			CU::Vector3f halfWidth{ box.halfExtents.x, box.halfExtents.y, box.halfExtents.z };

			Engine::GetInstance().GetDebugDrawer().DrawCube3D(pos, rot.EulerAngles(), halfWidth, 0, color);
		}
	}
	break;
	case physx::PxGeometryType::eCONVEXMESH: break;
	case physx::PxGeometryType::eTRIANGLEMESH: break;
	case physx::PxGeometryType::eHEIGHTFIELD: break;
	case physx::PxGeometryType::eGEOMETRY_COUNT: break;
	case physx::PxGeometryType::eINVALID: break;
	default:;
	}*/
}
