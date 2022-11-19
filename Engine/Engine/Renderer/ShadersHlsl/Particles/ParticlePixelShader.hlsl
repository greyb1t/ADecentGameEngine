#include "ParticleStructs.hlsli"

PixelOutput main(GeometryToPixel input)
{
    PixelOutput result;

    float4 textureColor = ParticleTexture.Sample(DefaultSampler, input.myUV);
    result.myColor.rgba = textureColor.rgba * input.myColor.rgba;
    
    return result;
}