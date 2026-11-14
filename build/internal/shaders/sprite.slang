import utils;
import attributes;
import "vertex/sprite_instance.slang";
import shaderInputs;

// Material Descriptor Set
struct MaterialInfo
{
    [Color] float3 color;
    [UNorm16] float alphaClippingThreshold = 0.5;
    int8_t doAlphaClipping;
};
[[vk::binding(0, 2)]] ConstantBuffer<MaterialInfo> materialInfo;
[[vk::binding(1, 2)]] Sampler2D mainTex; // SRGB

[shader("vertex")]
VOut vsMain(SpriteInstanceInfo instance, int vertexID : SV_VertexID)
{
    VOut output;
    float4x4 model = transpose(instance.Model);

    float2 vertexCornerID = quadPositions[vertexID];

    float3 vertexPos = float3(vertexCornerID, 0);

    output.FragPosition = mul(cameraData.viewProj, mul(model, float4(vertexPos, 1.0)));
    output.Position = mul(model, float4(vertexPos, 1.0)).xyz;

    float3 rawNormal = float3(0,0,1);
    float3x3 normalMatrix = transpose(adjugate(float3x3(model)));
    output.Normal = normalize(mul(normalMatrix, rawNormal));
    output.Color = float3(1,1,1);
    output.UV = lerp(instance.UVMin, instance.UVMax, vertexCornerID);

    return output;
}

// Entry point
[shader("fragment")]
float4 fsMain(FIn input)
{
    float4 outColor = mainTex.Sample(input.UV);
    if(materialInfo.doAlphaClipping != 0)
    {
        if(outColor.a < materialInfo.alphaClippingThreshold)
            discard;
        else
            outColor.w = 1;
    }
    return outColor;
}