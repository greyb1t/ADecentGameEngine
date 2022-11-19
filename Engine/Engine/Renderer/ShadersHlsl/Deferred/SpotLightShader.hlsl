#include "DeferredStructs.hlsli"
#include "../PBRFunctions.hlsli"

cbuffer SpotLightBuffer : register(b1)
{
	float4x4 lightView;
	float4x4 lightProjection;
	float4 myPosition;
	float4 myDirection;
	float4 myColorAndIntensity;
	float myRange;
	float myInnerAngle;
	float myOuterAngle;
	float trash;
}

PixelOutput main(VertexToPixel input)
{
	float4 albedoData = albedoTexture.Sample(defaultSampler, input.myUV);

	if (albedoData.a == 0)
	{
		discard;
	}

	float3 albedo = albedoData.rgb;

	PixelOutput output;

	float4 worldPosition = positionTexture.Sample(defaultSampler, input.myUV);
	float3 normal = normalTexture.Sample(defaultSampler, input.myUV).xyz;
	float3 vertexNormal = vertexNormalTexture.Sample(defaultSampler, input.myUV).xyz;
	float4 material = materialTexture.Sample(defaultSampler, input.myUV);

	float metalness = material.r;
	float roughness = material.g;

	float ao = ambientOcclusionTexture.Sample(defaultSampler, input.myUV).r;
	float depth = depthTexture.Sample(defaultSampler, input.myUV).r;

	float3 toEye = normalize(FB_CameraPosition.xyz - worldPosition.xyz);

	float3 specularColor = lerp((float3)0.04, albedo, metalness);
	float3 diffuseColor = lerp((float3)0.00, albedo, 1 - metalness);

	float3 spotLights = EvaluateSpotLight(
		diffuseColor,
		specularColor,
		normal,
		roughness,
		myColorAndIntensity.rgb,
		myColorAndIntensity.w,
		myRange,
		myPosition.xyz,
		myDirection.xyz,
		myOuterAngle,
		myInnerAngle,
		toEye.xyz,
		worldPosition.xyz);

	float4 worldToLightView = mul(lightView, worldPosition);
	float4 lightViewToLightProj = mul(lightProjection, worldToLightView);

	float2 projectedTexCoord;
	projectedTexCoord.x = lightViewToLightProj.x / lightViewToLightProj.w / 2.f + 0.5f;
	projectedTexCoord.y = -lightViewToLightProj.y / lightViewToLightProj.w / 2.f + 0.5f;

	if (saturate(projectedTexCoord.x) == projectedTexCoord.x &&
		saturate(projectedTexCoord.y) == projectedTexCoord.y)
	{
		const float shadowBias = 0.0005f;

		float shadow = 0.f;

		float viewDepth = (lightViewToLightProj.z / lightViewToLightProj.w) - shadowBias;

		float sampleDepth = ShadowTexture.Sample(pointSampler, projectedTexCoord).r;

		if (sampleDepth < viewDepth)
		{
			spotLights *= shadow;
		}
	}


	float3 radiance = spotLights;

	// NOTE: Removing LinearToGamma fixes the differences 
	// between forward and deferred rendering
	output.myColor.rgb = /*LinearToGamma*/(radiance);
	output.myColor.a = 1.0f;

	return output;
}