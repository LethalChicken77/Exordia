#include "attributes.slang"

struct VertexData
{
    uint VertexID : SV_VertexID;
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float4 Tangent : TANGENT;
    [UNorm8] float3 Color : COLOR;
    float2 UV : TEXCOORD0;
};