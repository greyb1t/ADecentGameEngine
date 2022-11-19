struct PixelOutput
{
	float4 myColor : SV_TARGET;
};

// MUST BE SAME AS IN FullscreenStructs.hlsli
struct VertexToPixel
{
	float4 myPosition : SV_POSITION;
	float2 myUV : UV;
};

cbuffer FrameBuffer : register(b0)
{
	float4x4 toCamera;
	float4x4 toProjection;
	float4 FB_CameraPosition;
}

TextureCube environmentTexture : register(t0);
Texture2D positionTexture : register(t1);
Texture2D albedoTexture : register(t2);
Texture2D normalTexture : register(t3);
Texture2D vertexNormalTexture : register(t4);
Texture2D materialTexture : register(t5);
Texture2D ambientOcclusionTexture : register(t6);
Texture2D depthTexture : register(t7);

Texture2D ShadowTexture : register(t9);
TextureCube ShadowTextureCube : register(t10);

SamplerState defaultSampler : register(s0);
SamplerState pointSampler : register(s1);