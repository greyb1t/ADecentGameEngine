struct VertexInput
{
	float4 myPosition : POSITION;
	float4 myColor : COLOR;

	float4 myNormal : NORMAL;
	float4 myTangent : TANGENT;
	float4 myBinormal : BINORMAL;

	float4 myBones : BONES;
	float4 myWeights : WEIGHTS;

	float2 myUV : UV;
};

struct VertexOutput
{
	float4 myPosition : SV_POSITION;
	float4 myColor : COLOR;

	float4 myNormal : NORMAL;
	float4 myTangent : TANGENT;
	float4 myBinormal : BINORMAL;

	float4 myWorldPosition : WORLD_POS;

	float2 myUV : UV;
};

struct PixelOutput
{
	float4 myColor : SV_TARGET;
};

cbuffer FrameBuffer : register(b0)
{
	float4x4 toCamera;
	float4x4 toProjection;

	float4 myCameraPosition;

	float4 toDirectionalLightDirection;
	float4 directionalLightColor;
}

cbuffer ObjectBuffer : register(b1)
{
	float4x4 toWorld;

	float2 myUVScale;

	unsigned int myPointLightCount;
	unsigned int mySpotLightCount;

	struct PointLightData
	{
		float4 myPosition;
		float4 myColorAndIntensity;
		float myRange;
		float3 garbage;
	} myPointLights[8];

	struct SpotLightData
	{
		float4 myPosition;
		float4 myDirection;
		float4 myColorAndIntensity;
		float myRange;
		float myInnerAngle;
		float myOuterAngle;
		float trash;
	} mySpotLights[8];
}

cbuffer AnimationBuffer : register(b2)
{
	float4x4 myBones[128];
	int OB_HasBones;
	int myGarbage1;
	int myGarbage2;
	int myGarbage3;
}

TextureCube environmentTexture : register(t0);
Texture2D albedoTexture : register(t1);
Texture2D normalTexture : register(t2);
Texture2D materialTexture : register(t3);
SamplerState defaultSampler : register(s0);