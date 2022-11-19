struct PixelOutput
{
	float4 myColor : SV_TARGET;
};

struct VertexInput
{
	float4 myPosition : POSITION;

	float4 myVxColor : COLOR;
	//float4 myColor1 : COLOR1;
	//float4 myColor2 : COLOR2;
	//float4 myColor3 : COLOR3;

	float2 myUV : UV;
	//float2 myUV1 : UV1;
	//float2 myUV2 : UV2;
	//float2 myUV3 : UV3;

	// binormal and shit?
	//float3
};

struct VertexToPixel
{
	float4 myPosition : SV_POSITION;

	float4 myColor : COLOR;
	//float4 myColor1 : COLOR1;
	//float4 myColor2 : COLOR2;
	//float4 myColor3 : COLOR3;

	float2 myUV : UV;

	//float2 myUV1 : UV1;
	//float2 myUV2 : UV2;
	//float2 myUV3 : UV3;

	// binormal and shit?
	//float3
};

cbuffer FrameBuffer : register(b0)
{
	float4x4 FB_ToCamera;
	float4x4 FB_ToProjection;
	float4 FB_CameraPosition;
	float FB_NearPlane;
	float FB_FarPlane;
	int FB_RenderMode;
}

cbuffer ObjectBuffer : register(b1)
{
	float4x4 OB_ToWorld;
}

SamplerState defaultSampler : register(s0);
Texture2D fontTexture : register(t0);