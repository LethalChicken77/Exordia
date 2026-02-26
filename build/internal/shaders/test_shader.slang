import utils;

// Camera Descriptor Set
struct CameraUbo
{
    row_major float4x4 view;
    row_major float4x4 invView;
    row_major float4x4 projection;
    column_major float4x4 viewProj;
};
[[vk::binding(0, 0)]] ConstantBuffer<CameraUbo> cameraData;

struct VertexData
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float4 Tangent : TANGENT;
    float3 Bitangent : BITANGENT;
    float3 Color : COLOR;
    float2 UV : TEXCOORD0;
};

struct InstanceData
{
    float4 Model0 : LOCATION6;
    float4 Model1 : LOCATION7;
    float4 Model2 : LOCATION8;
    float4 Model3 : LOCATION9;
};

struct VOut
{
    float4 FragPosition : SV_POSITION;
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float4 Tangent : TANGENT;
    float3 Bitangent : BITANGENT;
    float3 Color : COLOR;
    float2 UV : TEXCOORD0;
};

struct amogus
{
    float hi; // :D
};
// Material Descriptor Set
struct MaterialInfo
{
    float4 color;
    half3 normal;
    int8_t boolval;
    half3x3 smallmat;
    int8_t[12] twelve;
    amogus sus;
};
[[vk::binding(0, 2)]] ConstantBuffer<MaterialInfo> materialInfo;

[shader("vertex")]
VOut vsMain(VertexData vertex, InstanceData instance)
{
    VOut output;
    float4x4 model = transpose(float4x4(instance.Model0, instance.Model1, instance.Model2, instance.Model3));
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
    return float4(materialInfo.color.xyz, 1.0);
}