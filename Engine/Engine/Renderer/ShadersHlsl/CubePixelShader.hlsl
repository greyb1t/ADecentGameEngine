#include "CubeShaderStructs.hlsli"

PixelOutput main(VertexOutput aInput)
{
	PixelOutput pixelOutput;

	const float4 textureColor = albedoTexture.Sample(defaultSampler, aInput.myUV.xy).rgba;

	//pixelOutput.myColor = aInput.myColor;
	pixelOutput.myColor = textureColor;

	return pixelOutput;
}