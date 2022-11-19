struct VertexToPixel
{
	float4 myPosition : SV_POSITION;
	float4 myColor : COLOR;

	float4 myNormal : NORMAL;
	float4 myTangent : TANGENT;
	float4 myBiNormal : BINORMAL;

	float4 myWorldPosition : WORLD_POS;

	float2 myUV : UV;
};

/*
cbuffer FrameBuffer : register(b0)
{
	float4x4 toCamera;
	float4x4 toProjection;

	float4 myCameraPosition;
}
*/

cbuffer ObjectBuffer : register(b1)
{
	float4x4 toWorld;

	float2 myUVScale;
	float2 myTrashh;
}

SamplerState defaultSampler : register(s0);
Texture2D albedoTexture : register(t1);
Texture2D normalTexture : register(t2);
Texture2D materialTexture : register(t3);