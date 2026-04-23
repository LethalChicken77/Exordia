import attributes;

public struct VertexData
{
    public uint VertexID : SV_VertexID;
    public float3 Position : POSITION;
    public float3 Normal : NORMAL;
    public float4 Tangent : TANGENT;
    [UNorm8] public float3 Color : COLOR;
    public float2 UV : TEXCOORD0;
};