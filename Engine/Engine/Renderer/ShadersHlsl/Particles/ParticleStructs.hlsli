struct VertexInput
{
	float4 myPosition : POSITION;
	float4 myVelocity : VELOCITY;
	float4 myColor : COLOR;
	float2 mySize : SIZE;
	float myLifetime : LIFETIME;
	float myDistanceToCamera : DISTANCE;
	float myEmissiveStrength : EMISSIVE;
};

struct VertexToGeometry
{
	float4 myPosition : POSITION;
	float4 myColor : COLOR;
	float2 mySize : SIZE;
	float myEmissiveStrength : EMISSIVE;
};

struct GeometryToPixel
{
	float4 myPosition : SV_POSITION;
	float4 myColor : COLOR;
	float2 myUV : UV;
	float myEmissiveStrength : EMISSIVE;
};

struct PixelOutput
{
	float4 myColor : SV_TARGET;
};

cbuffer FrameBuffer : register(b0)
{
	float4x4 FB_ToCamera;
	float4x4 FB_ToProjection;
};

cbuffer ObjectBuffer : register(b1)
{
	float4x4 OB_ToWorld;
};

Texture2D ParticleTexture : register(t0);
SamplerState DefaultSampler : register(s0);