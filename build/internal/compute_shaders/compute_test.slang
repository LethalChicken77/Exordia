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
[vk::binding(0,0)] ConstantBuffer<ComputeParams> Params;

// Template for storage texture
[vk::binding(0,1)] RWTexture2D<float4> Result;

// Main entry point for shader
[shader("compute")] [numthreads(8,8,1)]
void csMain(ComputeInput sv) // Function name does not matter, must have the shader stage flag to be identified as an entry-point.
{
    int3 id = sv.id;
    Result[id.xy] = float4(id.x & id.y, (id.x & 15)/15.0, (id.y & 15)/15.0, 0.0);
}