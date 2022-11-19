#include "CubeShaderStructs.hlsli"

VertexOutput main(VertexInput aInput)
{
	VertexOutput vertexOutput;

	float4 vertexObjectPos = aInput.myPosition.xyzw;
	float4 vertexWorldPos = mul(toWorld, vertexObjectPos);
	float4 vertexViewPos = mul(toCamera, vertexWorldPos);
	float4 vertexProjectionPos = mul(toProjection, vertexViewPos);

	vertexOutput.myPosition = vertexProjectionPos;
	vertexOutput.myColor = aInput.myColor;
	vertexOutput.myUV = aInput.myUV;

	return vertexOutput;
}