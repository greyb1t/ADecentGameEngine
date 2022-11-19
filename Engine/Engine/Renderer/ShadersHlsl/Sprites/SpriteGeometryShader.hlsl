#include "SpriteStructs.hlsli"

[maxvertexcount(4)]
void main(
    point VertexToGeometry input[1],
    inout TriangleStream<GeometryToPixel> output
)
{
    VertexToGeometry inputVertex = input[0];

    const float ratio = myResolution.y / myResolution.x;

    const float2 offsets[4] =
    {
        float2(-1.0f, 1.0f),
        float2(1.0f, 1.0f),
        float2(-1.0f, -1.0f),
        float2(1.0f, -1.0f)
    };

    const float2 uvs[4] =
    {
        // Top left
        { inputVertex.myUVRect.x, inputVertex.myUVRect.y },

        // Top right
        { inputVertex.myUVRect.z, inputVertex.myUVRect.y },

        // Bottom left
        { inputVertex.myUVRect.x, inputVertex.myUVRect.w },

        // Bottom right
        { inputVertex.myUVRect.z, inputVertex.myUVRect.w }
    };

    for (unsigned int index = 0; index < 4; index++)
    {
        GeometryToPixel vertex;

        float2 center = inputVertex.myPosition.xy;

        float2 delta = offsets[index] * inputVertex.mySize;

        // delta -= inputVertex.myPivot;

        const float cosTheta = cos(inputVertex.myRotationRadians);
        const float sinTheta = sin(inputVertex.myRotationRadians);

        float2 rotatedDelta = float2(
            cosTheta * delta.x + sinTheta * delta.y,
            -sinTheta * delta.x + cosTheta * delta.y);

        rotatedDelta.x *= ratio;

        float2 corner = center + float2(rotatedDelta.x, rotatedDelta.y);

        vertex.myPosition = inputVertex.myPosition;
        vertex.myPosition.xy = corner;

        vertex.myColor = inputVertex.myColor;
        vertex.myUV = uvs[index];

        output.Append(vertex);
    }
}