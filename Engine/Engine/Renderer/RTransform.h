#pragma once

namespace Engine
{
	class RTransform
	{
	public:
		const Vec3f& GetPosition() const;
		void SetPosition(const Vec3f& aPosition);

		Vec3f GetRotationEuler() const;
		Quatf GetRotationQuat() const;
		void SetRotationEuler(const Vec3f& aRotation);
		void SetRotationQuaternion(const Quatf& aQuaternion);

		const Vec3f& GetScale() const;
		void SetScale(const Vec3f& aScale);
		void SetScale(const float aScale);

		Mat4f ToMatrix() const;

		// Just like the + operator, (maybe overload it?)
		RTransform Combine(const RTransform& aOther) const;

	private:
		Vec3f myPosition;
		Quatf myRotationQuat;
		Vec3f myScale = { 1.f, 1.f, 1.f };
	};
}