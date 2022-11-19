#include "FullscreenStructs.hlsli"

PixelOutput main(VertexToPixel input)
{
	PixelOutput result;

	float3 resource1 = FullscreenTexture1.Sample(DefaultSampler, input.myUV).rgb;
	float3 resource2 = FullscreenTexture2.Sample(DefaultSampler, input.myUV).rgb;

	// Additive
	{
		result.myColor.rgb = resource1 + resource2;
	}

	/*
	// Scaled
	{
		float3 scaledResource = resource1 * (1.0f - saturate(resource2));
		result.myColor.rgb = scaledResource + resource2;
	}
	*/

	/*
	// Luminance Based
	{
		float luminance = dot(resource1, float3(0.2126f, 0.7152f, 0.0722f));
		float3 scaledResource = resource2 * (1.0f - luminance);
		result.myColor.rgb = resource1 + scaledResource;
	}
	*/

	result.myColor.a = 1.0f;

	return result;
}