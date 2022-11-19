#include "SpriteStructs.hlsli"

PixelOutput main(GeometryToPixel input)
{
    PixelOutput result;

    float4 textureColor = SpriteTexture.Sample(DefaultSampler, input.myUV);
    //result.myColor.rgba = textureColor.rgba * input.myColor.rgba;
    
    // result.myColor.rgba = float4(1, 0, 0, 1);
    // result.myColor.rgba = float4(textureColor * input.myColor.rgb, input.myColor.a);
    result.myColor.rgba = textureColor * input.myColor;
    
    return result;
}