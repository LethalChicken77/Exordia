import attributes;

public static const float2 quadPositions[6] =
{
    float2(0.0, 0.0),
    float2(1.0, 0.0),
    float2(1.0, 1.0),

    float2(0.0, 0.0),
    float2(1.0, 1.0),
    float2(0.0, 1.0)
};

public struct SpriteInstanceInfo
{
    public float4x4 Model : MODEL;
    public float2 Size;
    public float2 UVMin;
    public float2 UVMax;
    public float Left;
    public float Right;
    public float Top;
    public float Bottom;
    public uint8_t Tiled;
    
    public uint InstanceID : SV_InstanceID;
};