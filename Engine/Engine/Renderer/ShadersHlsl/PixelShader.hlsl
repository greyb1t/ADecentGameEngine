#include "ShaderStructs.hlsli"
#include "PBRFunctions.hlsli"

float3 PixelShader_Albedo(VertexOutput aInput)
{
	float3 albedo = albedoTexture.Sample(defaultSampler, aInput.myUV.xy).rgb;
	//return albedo;
	return /*GammaToLinear*/(albedo);
}

float3 PixelShader_Normal(VertexOutput aInput)
{
	// I dont know which is better, this one, or the one below
	// One is from earlier föreläsningar, this one is from later deferred rendering föreläsningar

	/*
	float3 normal = normalTexture.Sample(defaultSampler, aInput.myUV.xy).wyz;

	normal = 2.0f * normal - 1.0f;
	normal.z = sqrt(1 - saturate(normal.x * normal.x + normal.y * normal.y));
	normal = normalize(normal);

	float3x3 TBN = float3x3(
		normalize(aInput.myTangent.xyz),
		normalize(aInput.myBinormal.xyz),
		normalize(aInput.myNormal.xyz)
		);

	TBN = transpose(TBN);
	return normalize(mul(TBN, normal));
	*/

	float3 normal = float3(normalTexture.Sample(defaultSampler, aInput.myUV.xy).wy, 0);

	normal = 2.0 * normal - 1.0;
	normal.z = sqrt(1.0 - saturate(normal.x * normal.x + normal.y * normal.y));

	float3x3 TBN = float3x3(
		normalize(aInput.myTangent.xyz),
		normalize(aInput.myBinormal.xyz),
		normalize(aInput.myNormal.xyz)
		);

	normal = mul(normal.xyz, TBN);
	normal = normalize(normal);

	return normal.xyz;
}

float3 PixelShader_Material(VertexOutput aInput)
{
	return materialTexture.Sample(defaultSampler, aInput.myUV.xy).rgb;
}

float PixelShader_Metalness(VertexOutput aInput)
{
	return PixelShader_Material(aInput).r;
}

float PixelShader_PerceptualRoughness(VertexOutput aInput)
{
	return PixelShader_Material(aInput).g;
}

float PixelShader_Emissive(VertexOutput aInput)
{
	return PixelShader_Material(aInput).b;
}

float PixelShader_AmbientOcclusion(VertexOutput aInput)
{
	return normalTexture.Sample(defaultSampler, aInput.myUV.xy).b;
}

PixelOutput main(VertexOutput aInput)
{
	float3 toEye = normalize(myCameraPosition.xyz - aInput.myWorldPosition.xyz);
	float3 albedo = PixelShader_Albedo(aInput);
	float3 normal = PixelShader_Normal(aInput);
	float ambientOcclusion = PixelShader_AmbientOcclusion(aInput);
	float metalness = PixelShader_Metalness(aInput);
	float perceptualRoughness = PixelShader_PerceptualRoughness(aInput);
	float emissiveData = PixelShader_Emissive(aInput);

	float3 specularColor = lerp((float3)0.04, albedo, metalness);
	float3 diffuseColor = lerp((float3)0.00, albedo, 1 - metalness);

	float3 ambience = EvaluateAmbience(
		environmentTexture,
		normal,
		aInput.myNormal.xyz,
		toEye,
		perceptualRoughness,
		metalness,
		albedo,
		ambientOcclusion,
		diffuseColor,
		specularColor);

	float3 directionalLight = EvaluateDirectionalLight(
		diffuseColor,
		specularColor,
		normal,
		perceptualRoughness,
		directionalLightColor.xyz,
		toDirectionalLightDirection.xyz,
		toEye.xyz);

	float3 pointLights = 0.f;

	for (unsigned int i = 0; i < myPointLightCount; i++)
	{
		pointLights += EvaluatePointLight(
			diffuseColor,
			specularColor,
			normal,
			perceptualRoughness,
			myPointLights[i].myColorAndIntensity.rgb,
			myPointLights[i].myColorAndIntensity.w,
			myPointLights[i].myRange,
			myPointLights[i].myPosition.xyz,
			toEye.xyz,
			aInput.myWorldPosition.xyz);
	}

	float3 spotLights = 0.f;

	for (unsigned int s = 0; s < mySpotLightCount; s++)
	{
		spotLights += EvaluateSpotLight(
			diffuseColor,
			specularColor,
			normal,
			perceptualRoughness,
			mySpotLights[s].myColorAndIntensity.rgb,
			mySpotLights[s].myColorAndIntensity.w,
			mySpotLights[s].myRange,
			mySpotLights[s].myPosition.xyz,
			mySpotLights[s].myDirection.xyz,
			mySpotLights[s].myOuterAngle,
			mySpotLights[s].myInnerAngle,
			toEye.xyz,
			aInput.myWorldPosition.xyz);
	}

	float3 emissive = albedo * emissiveData;
	float3 radiance = ambience + directionalLight + pointLights + spotLights + emissive;

	PixelOutput pixelOutput;
	pixelOutput.myColor.rgb = LinearToGamma(radiance);
	pixelOutput.myColor.a = 1.0f;

	return pixelOutput;
}