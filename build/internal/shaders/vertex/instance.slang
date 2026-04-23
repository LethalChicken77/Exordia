import attributes;

public struct InstanceData
{
    public float4x4 Model : MODEL;
    // float4 Model0 : MODEL0;
    // float4 Model1 : MODEL1;
    // float4 Model2 : MODEL2;
    // float4 Model3 : MODEL3;
    public uint InstanceID : SV_InstanceID;
};