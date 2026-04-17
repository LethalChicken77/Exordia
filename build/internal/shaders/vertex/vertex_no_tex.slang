#include "attributes.slang"

struct VertexData
{
    uint VertexID : SV_VertexID;
    float3 Position : POSITION;
    [SNorm16] float2 Normal : NORMAL;
    [UNorm8] float3 Color : COLOR;
};