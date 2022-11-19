#pragma once

#include "UVRect.h"

struct ParticleVertex
{
	C::Vector4f myPosition{};
	C::Vector4f myVelocity{};
	C::Vector4f myColor{};
	Vec2f mySize{};
	float myLifetime = 0.f;
	float myDistanceToCamera = 0.f;
	float myEmissiveStrength = 0.f;
	float myRotationRadians = 0.f;

	static constexpr size_t ourElementCount = 8;
	static const D3D11_INPUT_ELEMENT_DESC ourInputElements[ourElementCount];
};

struct SpriteVertex
{
	// Each vertex is a point given to the geometry shader
	C::Vector4f myPosition{};
	C::Vector4f myColor = { 1.f, 1.f, 1.f, 1.f };
	UVRect myUVRect{};
	Vec2f mySize = { 1.f, 1.f };
	Vec2f myPivot = { 0.5f, 0.5f };
	Vec2f myOffset = {};
	float myRotationRadians = 0.f;

	static constexpr size_t ourElementCount = 7;
	static const D3D11_INPUT_ELEMENT_DESC ourInputElements[ourElementCount];
};

struct LineVertex
{
	LineVertex() = default;

	LineVertex(C::Vector4f aPosition, C::Vector4f aColor)
		: myPosition(aPosition),
		myColor(aColor)
	{
	}

	C::Vector4f myPosition{};
	C::Vector4f myColor{};

	static constexpr size_t ourElementCount = 2;
	static const D3D11_INPUT_ELEMENT_DESC ourInputElements[ourElementCount];
};

struct DefaultVertex
{
	DefaultVertex() = default;

	DefaultVertex(const C::Vector4f& aPosition, const C::Vector4f& aColor, const Vec2f& aUVs)
	{
		myPosition = aPosition;
		myColor = aColor;
		myUVs = aUVs;
	}

	C::Vector4f myPosition{};
	C::Vector4f myColor{};

	C::Vector4f myNormal{};
	C::Vector4f myTangent{};
	C::Vector4f myBinormal{};

	C::Vector4f myBones{};
	C::Vector4f myWeights{};

	Vec2f myUVs{};

	bool operator==(const DefaultVertex& aSecond) const
	{
		return memcmp(this, &aSecond, sizeof(DefaultVertex)) == 0; 
	}

	static constexpr size_t ourElementCount = 8;
	static const D3D11_INPUT_ELEMENT_DESC ourInputElements[ourElementCount];
};

struct TextVertex
{
	TextVertex() = default;

	TextVertex(const Vec3f& aPosition, const C::Vector4f& aColor, const Vec2f& aUV)
	{
		myPosition = C::Vector4f(aPosition.x, aPosition.y, aPosition.z, 1.f);
		myColor1 = aColor;
		myUV1 = aUV;
	}

	C::Vector4f myPosition{};
	C::Vector4f myColor1{};
	Vec2f myUV1{};

	static constexpr size_t ourElementCount = 3;
	static const D3D11_INPUT_ELEMENT_DESC ourInputElements[ourElementCount];
};