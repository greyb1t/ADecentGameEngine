#pragma once
class VFXParticleAttraction
{
public:
	static const Vec3f&	GetAttractionPoint();
	static void			SetAttractionPoint(const Vec3f& aAttractionPoint);
private:
	static inline Vec3f myAttractionPoint = { 0, 0, 0 };
};

