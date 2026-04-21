import utils;
import shaderInputs;
import "vertex/vertex_full.slang";
import "vertex/instance.slang";
// #include "attributes.slang"

struct SubStruct
{
    float[8] wow;
}

struct Amogus
{
    int hi;
    bool bye;
    float3x3 matrix3;
    SubStruct amogusSub;
}

// Material Descriptor Set
struct MaterialInfo
{
// [[Color]] float4 color; // Interpret me as a color!
    float4 color;
    half3 normal;
    int test;
    float[24] floatArr;
    float4x4 matrix;
    float4x4[12] matArr;
    Amogus sus;
    Amogus[8] susses;
}
[[vk::binding(0, 2)]] ConstantBuffer<MaterialInfo> materialInfo;
[[vk::binding(1, 2)]] Sampler2D combinedImageSampler;
[[vk::binding(2, 2)]] Texture2D storageImage;
[[vk::binding(3, 2)]] SamplerState sampler;
[[vk::binding(4, 2)]] Buffer<float4> texelBuffer;
[[vk::binding(5, 2)]] RWBuffer<float4> texelStorageBuffer;
[[vk::binding(6, 2)]] RaytracingAccelerationStructure accelerationStructure;
[[vk::binding(7, 2)]] StructuredBuffer<Amogus> storageBuffer;
[[vk::binding(8, 2)]] RWStructuredBuffer<Amogus> rwstorageBuffer;
// [[vk::binding(0, 3)]] ParameterBlock<MaterialInfo> inlineMaterial;
Texture2D textures[];
// [[vk::binding(9, 2)]] 
// [[vk::input_attachment_index(0)]] SubpassInput<float4> inputAttachment;

[shader("vertex")]
VOut vsMain(VertexData vertex, InstanceData instance)
{
    VOut output;
    float4x4 model = transpose(instance.Model);
    // model = float4x4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
    output.FragPosition = mul(cameraData.viewProj, mul(model, float4(vertex.Position, 1.0)));
    output.Position = mul(model, float4(vertex.Position, 1.0)).xyz;

    float3x3 normalMatrix = transpose(inverse(float3x3(model)));
    output.Normal = normalize(mul(normalMatrix, vertex.Normal));
    output.Color = vertex.Color;
    output.UV = vertex.UV;

    return output;
}

// Entry point
[shader("fragment")]
float4 fsMain(VOut input)
{
    // return float4(float3(1,0,0), 1.0);
    return float4(materialInfo.color.xyz, 1.0);
}