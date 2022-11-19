struct VertexInput
{
    float4 myPosition : POSITION;
    float4 myColor : COLOR;
    float4 myUVRect : UV;
    float2 mySize : SIZE;
    float2 myPivot: PIVOT;
    float myRotationRadians : ROTATION;
};

struct VertexToGeometry
{
    float4 myPosition : POSITION;
    float4 myColor : COLOR;
    float4 myUVRect : UV;
    float2 mySize : SIZE;
    float2 myPivot : PIVOT;
    float myRotationRadians : ROTATION;
};

struct GeometryToPixel
{
    float4 myPosition : SV_POSITION;
    float4 myColor : COLOR;
    float2 myUV : UV;
};

struct PixelOutput
{
    float4 myColor : SV_TARGET;
};

cbuffer FrameBuffer : register(b0)
{
    float2 myResolution;
};

// cbuffer ObjectBuffer : register(b1)
// {
//     float4x4 OB_ToWorld;
// };

Texture2D SpriteTexture : register(t0);
SamplerState DefaultSampler : register(s0);