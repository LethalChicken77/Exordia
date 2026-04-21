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

struct FOut
{
    float4 Color : SV_Target;
    float Depth : SV_Depth;
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
    row_major float4x4 invProj;
    column_major float4x4 viewProj;
    float nearPlane;
    float farPlane;
    float yFOV;
    float xFOV;
    float aspectRatio;
    bool orthographic;
    bool reversedDepth;
    bool infiniteFarPlane;
};
[[vk::binding(0, 0)]] ConstantBuffer<CameraUbo> cameraData;

float GetDepth(float depthSample)
{
    if (cameraData.orthographic != 0)
    {
        return lerp(cameraData.nearPlane, cameraData.farPlane, depthSample);
    }
    float4 clip = float4(0.0, 0.0, depthSample, 1.0);

    float4 view = mul(cameraData.invProj, clip);
    view = view / view.w;

    return -view.z;
}

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