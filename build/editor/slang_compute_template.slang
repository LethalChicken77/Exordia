struct ComputeInput
{
    uint3 id : SV_DispatchThreadID; // Global thread ID. This one is the most useful!
    uint3 groupThreadID : SV_GroupThreadID; // Thread ID within its group
    uint3 groupID : SV_GroupID; // ID of the thread's group
    uint groupIndex : SV_GroupIndex; // Flattened local index within its group
}

// Template for shader parameters. Used like material parameters.
struct ComputeParams
{
    uint Width;
    uint Height;
}
ConstantBuffer<ComputeParams> Params;

// Template for compute data, ie. simulation particles
struct ComputeData
{

}
RWStructuredBuffer<ComputeData> Data;

// Template for storage texture
RWTexture2D<float4> Result;

// Main entry point for shader
[numthreads(8,8,1)]
[shader("compute")] void csMain(ComputeInput sv) // Function name does not matter, must have [shader("compute")] to be identified as an entry-point.
{
    int3 id = sv.id;
    Result[id.xy] = float4(id.x & id.y, (id.x & 15)/15.0, (id.y & 15)/15.0, 0.0); // From Unity's template compute shader
}