import attributes;

public struct VertexData
{
    public uint VertexID : SV_VertexID;
    public float3 Position : POSITION;
    [SNorm16] public float2 Normal : NORMAL;
    [UNorm8] public float3 Color : COLOR;
};