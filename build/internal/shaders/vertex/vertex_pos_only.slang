import attributes;
import vertex_attributes;

[VertexLayout] public struct VertexData
{
    public uint VertexID : SV_VertexID;
    public float3 Position : POSITION;
};