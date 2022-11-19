#include "ParticleStructs.hlsli"

VertexToGeometry main(VertexInput input)
{
	VertexToGeometry result;

	float4 vertexObjectPos = input.myPosition;
	float4 vertexWorldPos = mul(OB_ToWorld, vertexObjectPos);
	float4 vertexViewPos = mul(FB_ToCamera, vertexWorldPos);

	result.myPosition = vertexViewPos;
	result.myColor = float4(input.myColor.rgb * input.myEmissiveStrength, input.myColor.a);
	result.mySize = input.mySize;
	result.myEmissiveStrength = input.myEmissiveStrength;

	return result;
}