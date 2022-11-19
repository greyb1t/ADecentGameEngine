#include "FullscreenStructs.hlsli"

PixelOutput main(VertexToPixel input)
{
	PixelOutput result;

	float3 resource = FullscreenTexture1.Sample(DefaultSampler, input.myUV).rgb;

	/*
	// Gain, no cutoff
	{
		float luminance = dot(resource, float3(0.2126f, 0.7152f, 0.0722f));
		result.myColor.rgb = resource * pow(luminance, 5);
	}
	*/

	/*
	// Flat cutoff
	{
		float luminance = dot(resource, float3(0.2126f, 0.7152f, 0.0722f));
		float cutOff = 0.8f;
		if (luminance >= cutOff)
		{
			result.myColor.rgb = resource;
		}
		else
		{
			result.myColor.rgb = 0;
		}
	}
	*/

	/*
	// Gain, cutoff
	{
		float luminance = dot(resource, float3(0.2126f, 0.7152f, 0.0722f));
		float cutOff = 0.5f;
		luminance = saturate(luminance - cutOff);
		result.myColor.rgb = resource * luminance * (1.0f / cutOff);
	}
	*/

	// Flat cutoff with smooth edges
	{
		float luminance = dot(resource, float3(0.2126f, 0.7152f, 0.0722f));
		float cutOff = 0.8f;
		if (luminance >= cutOff)
		{
			result.myColor.rgb = resource;
		}
		else if (luminance >= (cutOff * 0.5f))
		{
			float fade = luminance / cutOff;
			fade = pow(fade, 5);
			result.myColor.rgb = resource * fade;
		}
		else
		{
			result.myColor.rgb = 0;
		}
	}

	result.myColor.a = 1.0f;

	return result;
}