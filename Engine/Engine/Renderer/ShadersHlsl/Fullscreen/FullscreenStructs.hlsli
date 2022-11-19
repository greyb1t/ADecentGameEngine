struct VertexInput
{
	unsigned int myIndex : SV_VertexID;
};

struct VertexToPixel
{
	float4 myPosition : SV_POSITION;
	float2 myUV : UV;
};

struct PixelOutput
{
	float4 myColor : SV_TARGET;
};

SamplerState DefaultSampler : register(s0);
Texture2D FullscreenTexture1 : register(t0);
Texture2D FullscreenTexture2 : register(t1);

// the number 4 constant because textures render targets are a quarter of the resolution
static const float2 Resolution = float2(1280.f, 720.f) / 4.f;
static const float GaussianKernel5[5] = { 0.06136f, 0.24477f, 0.38774f, 0.24477f, 0.06136f };