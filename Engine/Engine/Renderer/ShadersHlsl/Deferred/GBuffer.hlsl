#include "GBufferStructs.hlsli"
#include "../PBRFunctions.hlsli"

struct GBufferOutput
{
	float4 myWorldPosition : SV_TARGET0;
	float4 myAlbedo : SV_TARGET1;
	float4 myNormal2 : SV_TARGET2;
	float4 myVertexNormal : SV_TARGET3;
	float3 myMaterial : SV_TARGET4;
	float myAmbientOcclusion : SV_TARGET5;
	float myDepth : SV_TARGET6;
};

GBufferOutput main(VertexToPixel input)
{
	float2 scaledUV = input.myUV * myUVScale;
	float3 albedo = /*GammaToLinear*//*LinearToGamma*/(albedoTexture.Sample(defaultSampler, scaledUV).rgb);

	float3 normal = normalTexture.Sample(defaultSampler, scaledUV).wyz;
	float ambientOcclusion = normal.b;

	normal = 2.0f * normal - 1.0f;
	normal.z = sqrt(1 - saturate(normal.x * normal.x + normal.y * normal.y));
	normal = normalize(normal);

	float3x3 TBN = float3x3(
		normalize(input.myTangent.xyz),
		normalize(input.myBiNormal.xyz),
		normalize(input.myNormal.xyz)
		);

	TBN = transpose(TBN);
	float3 pixelNormal = normalize(mul(TBN, normal));
	float3 material = materialTexture.Sample(defaultSampler, scaledUV).rgb;

	GBufferOutput output;
	
	output.myWorldPosition = input.myWorldPosition;
	output.myAlbedo = float4(albedo, 1.0f);
	output.myNormal2 = float4(pixelNormal, 1.0f);
	output.myVertexNormal = float4(input.myNormal.xyz, 1.0f);
	output.myMaterial = material;

	// what is input.myDepth
	//output.myDepth = input.myDepth;
	output.myDepth = 1.f;

	output.myAmbientOcclusion = ambientOcclusion;

	return output;
}