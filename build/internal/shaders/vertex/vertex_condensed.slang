import attributes;

public struct VertexData
{
    public uint VertexID : SV_VertexID;
    public float3 Position : POSITION;
    [SNorm16] public float2 Normal : NORMAL;
    [SNorm16] public float3 Tangent : TANGENT;
    [UNorm8] public float3 Color : COLOR;
    [Half] public float2 UV : TEXCOORD0;
};