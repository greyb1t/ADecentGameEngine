#pragma once

namespace Engine
{
	class LiteTransform
	{
	public:
		virtual ~LiteTransform() = default;

		void SetPositionLocal(const Vec3f& aPos);
		void SetRotationLocal(const Quatf& aQuaternion);
		void SetScaleLocal(const Vec3f& aScale);
		void SetScaleLocal(const float aScale);

		const Vec3f& GetPositionLocal() const;
		const Quatf& GetRotationLocal() const;
		const Vec3f& GetScaleLocal() const;

		const Vec3f	LocalForward() const;
		const Vec3f	LocalRight() const;
		const Vec3f	LocalUp() const;

		Mat4f GetMatrix() const;

	protected:
		Vec3f myPosition;
		Quatf myRotation;
		Vec3f myScale = { 1, 1, 1 };
	};
}