import attributes;

module shaderInputs;

public struct VOut
{
    public float3 Position : POSITION;
    public float3 Normal : NORMAL;
    public float4 Tangent : TANGENT;
    public float3 Bitangent : BITANGENT;
    public float3 Color : COLOR;
    public float2 UV : TEXCOORD0;
    
    public float4 FragPosition : SV_Position;
};

public struct FIn
{
    public float3 Position : POSITION;
    public float3 Normal : NORMAL;
    public float4 Tangent : TANGENT;
    public float3 Bitangent : BITANGENT;
    public float3 Color : COLOR;
    public float2 UV : TEXCOORD0;
    
    public float4 FragPosition : SV_POSITION;
    public bool isFrontFace : SV_IsFrontFace;
};

public struct FOut
{
    public float4 Color : SV_Target;
    public float Depth : SV_Depth;
};

public struct PushConstants
{
    public int objectID;
};
[[vk::push_constant]]
public PushConstants pushConstants;

// Camera Descriptor Set
public struct CameraUbo
{
    public row_major float4x4 view;
    public row_major float4x4 invView;
    public row_major float4x4 projection;
    public row_major float4x4 invProj;
    public column_major float4x4 viewProj;
    public float nearPlane;
    public float farPlane;
    public float yFOV;
    public float xFOV;
    public float aspectRatio;
    public bool orthographic;
    public bool reversedDepth;
    public bool infiniteFarPlane;
};
[[vk::binding(0, 0)]] public ConstantBuffer<CameraUbo> cameraData;

export float GetDepth(float depthSample)
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
public enum LightType
{
    DIRECTIONAL = 0,
    POINT = 1
};
public struct Light
{
    public float3 position;
    public int type;
    public float3 color;
    public float intensity;
};
public struct GlobalData
{
    public float3 ambientLight;
    public int numLights;
    public Light lights[128];
};
[vk::binding(0, 1)] public ConstantBuffer<GlobalData> globalData;