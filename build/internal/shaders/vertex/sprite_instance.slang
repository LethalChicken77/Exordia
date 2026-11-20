import attributes;
import vertex_attributes;

public static const float2 quadPositions[6] =
{
    float2(0.0, 0.0),
    float2(1.0, 1.0),
    float2(1.0, 0.0),

    float2(0.0, 0.0),
    float2(0.0, 1.0),
    float2(1.0, 1.0)
};

[InstanceLayout] public struct SpriteInstanceInfo
{
    public float4x4 Model : MODEL;
    public float2 Size;
    public float2 Pivot;
    public float2 UVMin;
    public float2 UVMax;
    public float4 Border;
    public float2 TextureSize;
    public float PixelsPerUnit;
    public int TextureIndex; // TODO: Implement with bindless textures
    public uint Tiled;
    
    public uint InstanceID : SV_InstanceID;
};