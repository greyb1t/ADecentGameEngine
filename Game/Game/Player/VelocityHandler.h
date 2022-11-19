#pragma once
class VelocityHandler
{
public:
	void			Set(const Vec3f& aVelocity);
	void			SetY(const float aY);
	void			Add(const Vec3f& aVelocity);
	const Vec3f&	Get() const;
	Vec3f&			Ref();
private:
	Vec3f myVelocity;
};

