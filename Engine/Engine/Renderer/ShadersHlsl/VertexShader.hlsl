#include "ShaderStructs.hlsli"

VertexOutput main(VertexInput aInput)
{
    VertexOutput vertexOutput;

    float4 vertexWorldPos;

    if (OB_HasBones == 1)
    {
        float4 boneWeights = aInput.myWeights;
        float4 BoneIds = aInput.myBones;

        // Daniel version
        // This seems to not work
        // float4 skinnedPos = aInput.myPosition;

        // assimp version
        float4 skinnedPos = 0;

        skinnedPos += boneWeights.x * mul(myBones[(uint) BoneIds.x], aInput.myPosition);

        skinnedPos += boneWeights.y * mul(myBones[(uint) BoneIds.y], aInput.myPosition);

        skinnedPos += boneWeights.z * mul(myBones[(uint) BoneIds.z], aInput.myPosition);

        skinnedPos += boneWeights.w * mul(myBones[(uint) BoneIds.w], aInput.myPosition);

        /*
        skinnedPos += boneWeights.x * mul(aInput.myPosition, myBones[(uint) BoneIds.x]);

        skinnedPos += boneWeights.y * mul(aInput.myPosition, myBones[(uint) BoneIds.y]);

        skinnedPos += boneWeights.z * mul(aInput.myPosition, myBones[(uint) BoneIds.z]);

        skinnedPos += boneWeights.w * mul(aInput.myPosition, myBones[(uint) BoneIds.w]);
        */

        // Move into correct position in world space
        vertexWorldPos = mul(toWorld, skinnedPos);
    }
    else
    {
        float4 vertexObjectPos = aInput.myPosition.xyzw;
        vertexWorldPos = mul(toWorld, vertexObjectPos);
    }


    float4 vertexViewPos = mul(toCamera, vertexWorldPos);
    float4 vertexProjectionPos = mul(toProjection, vertexViewPos);

    float3x3 toWorldRotation = (float3x3) toWorld;
    float3 vertexWorldNormal = mul(toWorldRotation, aInput.myNormal.xyz);
    float3 vertexWorldTangent = mul(toWorldRotation, aInput.myTangent.xyz);
    float3 vertexWorldBinormal = mul(toWorldRotation, aInput.myBinormal.xyz);

    vertexOutput.myNormal = float4(vertexWorldNormal, 0);
    vertexOutput.myTangent = float4(vertexWorldTangent, 0);
    vertexOutput.myBinormal = float4(vertexWorldBinormal, 0);

    vertexOutput.myPosition = vertexProjectionPos;
    vertexOutput.myColor = aInput.myColor;
    vertexOutput.myUV = aInput.myUV;

    // Send the vertex world pos to pixel shader and let it interpolate it
    vertexOutput.myWorldPosition = vertexWorldPos;

    return vertexOutput;
}