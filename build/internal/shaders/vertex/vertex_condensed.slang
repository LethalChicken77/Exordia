#include "attributes.slang"

struct VertexData
{
    uint VertexID : SV_VertexID;
    float3 Position : POSITION;
    [SNorm8] float2 Normal : NORMAL;
    [SNorm8] float3 Tangent : TANGENT;
    uint8_t3 Color : COLOR;
    half2 UV : TEXCOORD0;
};