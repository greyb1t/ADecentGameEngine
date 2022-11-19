#include "FullscreenStructs.hlsli"

PixelOutput main(VertexToPixel input)
{
	PixelOutput result;

	float texelSize = 1.0f / Resolution.y;
	float3 blurColor = 0;

	unsigned int kernelSize = 5;

	float start = (((float)(kernelSize)-1.0f) / 2.0f) * -1.f;

	for (unsigned int idx = 0; idx < kernelSize; idx++)
	{
		float2 uv = input.myUV + float2(0.0f, texelSize * (start + (float)idx));
		float3 resource = FullscreenTexture1.Sample(DefaultSampler, uv).rgb;
		blurColor += resource * GaussianKernel5[idx];
	}

	result.myColor.rgb = blurColor;
	result.myColor.a = 1.0f;

	return result;
}