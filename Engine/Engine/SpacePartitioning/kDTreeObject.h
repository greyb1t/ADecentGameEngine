#pragma once

// TODO: this should be replaced by a template typename T

class kDTreeObject
{
public:
	kDTreeObject(const Vec3f& aPosition, const float aHalfSize);

	const Vec3f& GetPosition() const;
	void SetPosition(const Vec3f& aPosition);

	float GetHalfSize() const;

	void Render();

	void SetLitUp();

	void ResetLittingUpState();

private:
	Vec3f myPosition;

	// TODO: make Vec2
	float myHalfSize = 0.f;

	// Tga2D::CSprite* mySprite = nullptr;
};