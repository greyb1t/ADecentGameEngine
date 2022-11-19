#include "FullscreenStructs.hlsli"

VertexToPixel main(VertexInput input)
{
	float4 pos[3] = 
	{
		float4(-1.f, -1.f, 0.f, 1.f),
		float4(-1.f, 3.f, 0.f, 1.f),
		float4(3.f, -1.f, 0.f, 1.f)
	};

	float2 uv[3] =
	{
		float2(0.f, 1.0f),
		float2(0.f, -1.0f),
		float2(2.f, 1.0f)
	};

	VertexToPixel returnValue;
	returnValue.myPosition = pos[input.myIndex];
	returnValue.myUV = uv[input.myIndex];
	return returnValue;
}