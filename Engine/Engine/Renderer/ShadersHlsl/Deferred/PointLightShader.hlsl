#include "DeferredStructs.hlsli"
#include "../PBRFunctions.hlsli"

cbuffer PointLightBuffer : register(b1)
{
	float4x4 myLightViews[6];
	float4x4 lightProjection;

	float4 myPointLightPosition;
	float4 myColorAndIntensity;
	float myRange;
	float3 garbage;
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

	float3 specularColor = lerp((float3) 0.04, albedo, metalness);
	float3 diffuseColor = lerp((float3) 0.00, albedo, 1 - metalness);

	float3 pointLights = EvaluatePointLight(
		diffuseColor,
		specularColor,
		normal,
		roughness,
		myColorAndIntensity.rgb,
		myColorAndIntensity.w,
		myRange,
		myPointLightPosition.xyz,
		toEye.xyz,
		worldPosition.xyz);

	for (int i = 0; i < 6; i++)
	{
		float4 worldToLightView = mul(myLightViews[i], worldPosition);
		float4 lightViewToLightProj = mul(lightProjection, worldToLightView);

		float3 projectedTexCoord;
		projectedTexCoord.x = lightViewToLightProj.x / lightViewToLightProj.w / 2.f + 0.5f;
		projectedTexCoord.y = -lightViewToLightProj.y / lightViewToLightProj.w / 2.f + 0.5f;
		projectedTexCoord.z = lightViewToLightProj.z / lightViewToLightProj.w / 2.f + 0.5f;

		// NOTE: Doing this while rendering the depth map creates 
		// very odd artifacts at the camera frustum lines and such
		// BUT, if I do not have it and try and render the calculated viewDepth
		// it will be FUCKED, so we MUST have it here
		if (saturate(projectedTexCoord.x) == projectedTexCoord.x &&
			saturate(projectedTexCoord.y) == projectedTexCoord.y &&
			// Z check must be here to avoid projection in the opposite direction making a "mirror"
			saturate(projectedTexCoord.z) == projectedTexCoord.z)
		{
			/*
				This was my way of rendering the depth map to
				ensure I correctly sample from the cube texture

				float x = InverseLerp(0.981f, 1.f, sampleDepth2);
				output.myColor = float4(x, 0, 0, 1);
				return output;
			*/

			const float shadowBias = 0.0005f;

			float shadow = 0.f;

			// cannot see these visually, bcuz they suck
			float viewDepth = (lightViewToLightProj.z / lightViewToLightProj.w) - shadowBias;

			/*
				This was my way of rendering the calculated viewDepth
				to ensure that is correct

				float x = InverseLerp(0.981f, 1.f, viewDepth);
				output.myColor = float4(x, 0, 0, 1);
				return output;
			*/

			float3 lightToPixel = worldPosition.xyz - myPointLightPosition.xyz;
			float sampleDepth = ShadowTextureCube.Sample(pointSampler, lightToPixel).r;

			if (sampleDepth < viewDepth)
			{
				pointLights *= shadow;
			}

			/*
			float3 lightToPixel = worldPosition.xyz - myPointLightPosition.xyz;

			// float shadow = 0.0;

			float shadowResult = 0.0;

			float bias = 0.05;
			float samples = 4.0;
			float offset = 0.1;
			for (float x = -offset; x < offset; x += offset / (samples * 0.5))
			{
				for (float y = -offset; y < offset; y += offset / (samples * 0.5))
				{
					for (float z = -offset; z < offset; z += offset / (samples * 0.5))
					{
						// float closestDepth = texture(depthMap, fragToLight + vec3(x, y, z)).r;
						float sampleDepth = ShadowTextureCube.Sample(pointSampler, lightToPixel + float3(x, y, z)).r;

						if (sampleDepth < viewDepth)
						{
							shadowResult += 1.0f;
							//pointLights *= shadow;
						}

						// closestDepth *= far_plane;   // undo mapping [0;1]
						//if (currentDepth - bias > closestDepth)
						//	shadow += 1.0;
					}
				}
			}

			shadowResult /= (samples * samples * samples);

			pointLights *= shadowResult;
			*/
		}
	}

	// Should not use emissive here, because it strengthens the emission each time...
	//float3 emissive = albedo * emissiveMask;

	float3 radiance = pointLights;

	// NOTE: Removing LinearToGamma fixes the differences 
	// between forward and deferred rendering
	output.myColor.rgb = /*LinearToGamma*/(radiance);
	output.myColor.a = 1.0f;

	return output;
}