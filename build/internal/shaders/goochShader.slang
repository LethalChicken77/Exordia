import utils;
import shading_models;
import colorspace; // TODO: Perform colorspace transforms in post-processing

import shaderInputs;
import "vertex/vertex_no_tex.slang";
import "vertex/instance.slang";

// Material Descriptor Set
struct MaterialInfo
{
    float4 coolColor;
    float4 warmColor;
    float4 outlineColor;
    float outlinePower;
    float roughness;
};
[[vk::binding(0, 2)]] ConstantBuffer<MaterialInfo> materialInfo;

[shader("vertex")]
VOut vsMain(VertexData vertex, InstanceData instance)
{
    VOut output;
    float4x4 model = transpose(instance.Model);
    output.FragPosition = mul(cameraData.viewProj, mul(model, float4(vertex.Position, 1.0)));
    output.Position = mul(model, float4(vertex.Position, 1.0)).xyz;

    float3x3 normalMatrix = transpose(adjugate(float3x3(model)));
    float3 rawNormal = UnpackOctahedral(vertex.Normal);
    output.Normal = normalize(mul(normalMatrix, rawNormal));

    float3 cameraPos = cameraData.view[3].xyz;
    float3 viewDir = normalize(output.Position.xyz - cameraPos);
    if (dot(output.Normal, viewDir) > 0)
    {
        output.Normal = normalize(output.Normal - viewDir * dot(output.Normal, viewDir));
    }

    output.Color = vertex.Color;

    return output;
}

// Entry point
[shader("fragment")]
float4 fsMain(VOut input)
{
    float3 normal = normalize(input.Normal);
    // float3 lightDir = normalize(float3(0.5, 0.4, 1));
    float3 cameraPos = cameraData.view[3].xyz;
    float3 viewDir = normalize(input.Position.xyz - cameraPos);
    // float3 lightColor = float3(1.0, 1.0, 1.0) * 4.0;
    float3 ambientColor = globalData.ambientLight;// = invLinearToSRGB(float3(0.04f, 0.08f, 0.2f));

    float3 F0 = float3(0.05f);

    Light currentLight = globalData.lights[0]; // TODO: Pass light direction in material properties
    // float3 lightDir = normalize(currentLight.position);

    float3 lightDir = float3(-1, 1, -0.5);
    lightDir = normalize(mul(cameraData.invView, float4(lightDir, 0.0)).xyz);

    float3 halfDir = 0.5 * (lightDir - viewDir);

    float3 lightColor = currentLight.color * currentLight.intensity;
    float3 coolColor = SRGBToLinear(materialInfo.coolColor.xyz);
    float3 warmColor = SRGBToLinear(materialInfo.warmColor.xyz);
    float4 outlineColor = float4(SRGBToLinear(materialInfo.outlineColor.xyz), materialInfo.outlineColor.w);//float3(1.0, 0.6, 0.1);
    float outlinePower = materialInfo.outlinePower;
    float3 outColor = goochBRDF(coolColor, warmColor, outlineColor, outlinePower, materialInfo.roughness, F0, lightColor, normal, lightDir, viewDir);
    // float3 outColor = PhongBRDF(albedo, 80, lightColor, ambientColor, normal, lightDir, viewDir);

    // outColor = OklabToLinear(outColor);
    // outColor = pow(outColor, 2.0);
    // outColor = ACEScgToLinear(outColor);
    // outColor = LinearToSRGB(outColor); // Gamma correction
    // outColor *= 0.5;
    // outColor = ACESFilmCurve(outColor);
    return float4(outColor, 1.0);
}