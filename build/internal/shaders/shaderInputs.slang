#include "attributes.slang"

struct VOut
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float4 Tangent : TANGENT;
    float3 Bitangent : BITANGENT;
    float3 Color : COLOR;
    float2 UV : TEXCOORD0;
    
    float4 FragPosition : SV_Position;
};

struct FIn
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float4 Tangent : TANGENT;
    float3 Bitangent : BITANGENT;
    float3 Color : COLOR;
    float2 UV : TEXCOORD0;
    
    float4 FragPosition : SV_POSITION;
    bool isFrontFace : SV_IsFrontFace;
};

struct PushConstants
{
    int objectID;
};
[[vk::push_constant]]
PushConstants pushConstants;

// Camera Descriptor Set
struct CameraUbo
{
    row_major float4x4 view;
    row_major float4x4 invView;
    row_major float4x4 projection;
    column_major float4x4 viewProj;
};
[[vk::binding(0, 0)]] ConstantBuffer<CameraUbo> cameraData;

// Global Descriptor Set
enum LightType
{
    DIRECTIONAL = 0,
    POINT = 1
};
struct Light
{
    float3 position;
    int type;
    float3 color;
    float intensity;
};
struct GlobalData
{
    float3 ambientLight;
    int numLights;
    Light lights[128];
};
[vk::binding(0, 1)] ConstantBuffer<GlobalData> globalData;