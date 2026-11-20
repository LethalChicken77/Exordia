import utils;
import attributes;
import "vertex/sprite_instance.slang";
import shaderInputs;

// Material Descriptor Set
struct MaterialInfo
{
    [Color] float3 color = float3(1,1,1);
    [UNorm16][Range(0,1)] float alphaClippingThreshold = 0.5;
    bool doAlphaClipping = true;
};
[[vk::binding(0, 2)]] ConstantBuffer<MaterialInfo> materialInfo;
[[vk::binding(1, 2)]] Sampler2D mainTex; // SRGB

void GetVertexInfo(int vertexID, SpriteInstanceInfo instance, out float3 vertexPos, out float2 vertexUV)
{
    uint hasLeft = instance.Border.x > 0.0;
    uint hasRight = instance.Border.y > 0.0;
    uint hasTop = instance.Border.z > 0.0;
    uint hasBottom = instance.Border.w > 0.0;

    float2 uvExtent = instance.UVMax - instance.UVMin;

    // Convert UV distances to world-space border widths.
    float left = instance.Border.x * instance.TextureSize.x / instance.PixelsPerUnit;
    float right = instance.Border.y * instance.TextureSize.x / instance.PixelsPerUnit;
    float top = instance.Border.z * instance.TextureSize.y / instance.PixelsPerUnit;
    float bottom = instance.Border.w * instance.TextureSize.y / instance.PixelsPerUnit;

    // Clamp individual borders to sprite dimensions.
    left = min(left, instance.Size.x);
    right = min(right, instance.Size.x);
    top = min(top, instance.Size.y);
    bottom = min(bottom, instance.Size.y);

    // Resolve overlapping borders proportionally.
    float horizontalSum = left + right;
    if(horizontalSum > instance.Size.x)
    {
        float scale = instance.Size.x / horizontalSum;
        left *= scale;
        right *= scale;
    }

    float verticalSum = top + bottom;
    if(verticalSum > instance.Size.y)
    {
        float scale = instance.Size.y / verticalSum;
        top *= scale;
        bottom *= scale;
    }

    // Convert world-space borders to normalized sprite coordinates.
    float leftNorm = left / instance.Size.x;
    float rightNorm = right / instance.Size.x;
    float topNorm = top / instance.Size.y;
    float bottomNorm = bottom / instance.Size.y;

    float xPositions[4];
    float yPositions[4];
    float xUV[4];
    float yUV[4];

    uint horizontalQuadCount = 1u + hasLeft + hasRight;

    // Horizontal divisions.
    uint xCount = 1;
    xPositions[0] = 0.0;
    xUV[0] = instance.UVMin.x;

    if(hasLeft != 0)
    {
        xPositions[xCount] = leftNorm;
        xUV[xCount++] = instance.UVMin.x + instance.Border.x;
    }

    if(hasRight != 0)
    {
        xPositions[xCount] = 1.0 - rightNorm;
        xUV[xCount++] = instance.UVMax.x - instance.Border.y;
    }

    xPositions[xCount] = 1.0;
    xUV[xCount] = instance.UVMax.x;

    // Vertical divisions (bottom to top).
    uint yCount = 1;
    yPositions[0] = 0.0;
    yUV[0] = instance.UVMin.y;

    if(hasBottom != 0)
    {
        yPositions[yCount] = bottomNorm;
        yUV[yCount++] = instance.UVMin.y + instance.Border.w;
    }

    if(hasTop != 0)
    {
        yPositions[yCount] = 1.0 - topNorm;
        yUV[yCount++] = instance.UVMax.y - instance.Border.z;
    }

    yPositions[yCount] = 1.0;
    yUV[yCount] = instance.UVMax.y;

    // Select the quad.
    uint quadID = vertexID / 6;
    uint quadX = quadID % horizontalQuadCount;
    uint quadY = quadID / horizontalQuadCount;

    float2 vertexCornerID = quadPositions[vertexID % 6];

    // Interpolate within the selected quad.
    float2 gridMin = float2(xPositions[quadX], yPositions[quadY]);
    float2 gridMax = float2(xPositions[quadX + 1], yPositions[quadY + 1]);

    float2 vertexCoordinate = lerp(gridMin, gridMax, vertexCornerID);

    vertexPos = float3(
        (vertexCoordinate - instance.Pivot) * instance.Size,
        0.0
    );

    float2 uvMin = float2(xUV[quadX], yUV[quadY]);
    float2 uvMax = float2(xUV[quadX + 1], yUV[quadY + 1]);

    vertexUV = lerp(uvMin, uvMax, vertexCornerID);
}

void GetTiledVertexInfo(int vertexID, SpriteInstanceInfo instance, out float3 vertexPos, out float2 vertexUV)
{
    uint hasLeft = instance.Border.x > 0.0;
    uint hasRight = instance.Border.y > 0.0;
    uint hasTop = instance.Border.z > 0.0;
    uint hasBottom = instance.Border.w > 0.0;

    // Convert UV distances to world-space border widths.
    float left = instance.Border.x * instance.TextureSize.x / instance.PixelsPerUnit;
    float right = instance.Border.y * instance.TextureSize.x / instance.PixelsPerUnit;
    float top = instance.Border.z * instance.TextureSize.y / instance.PixelsPerUnit;
    float bottom = instance.Border.w * instance.TextureSize.y / instance.PixelsPerUnit;

    // Clamp individual borders to sprite dimensions.
    left = min(left, instance.Size.x);
    right = min(right, instance.Size.x);
    top = min(top, instance.Size.y);
    bottom = min(bottom, instance.Size.y);

    // Resolve overlapping borders proportionally.
    float horizontalSum = left + right;
    if(horizontalSum > instance.Size.x)
    {
        float scale = instance.Size.x / horizontalSum;
        left *= scale;
        right *= scale;
    }

    float verticalSum = top + bottom;
    if(verticalSum > instance.Size.y)
    {
        float scale = instance.Size.y / verticalSum;
        top *= scale;
        bottom *= scale;
    }

    float leftNorm = left / instance.Size.x;
    float rightNorm = right / instance.Size.x;
    float topNorm = top / instance.Size.y;
    float bottomNorm = bottom / instance.Size.y;

    // Determine center region dimensions.
    float centerWidth = instance.Size.x - left - right;
    float centerHeight = instance.Size.y - top - bottom;

    float centerUVMinX = instance.UVMin.x + (hasLeft != 0 ? instance.Border.x : 0.0);
    float centerUVMaxX = instance.UVMax.x - (hasRight != 0 ? instance.Border.y : 0.0);
    float centerUVMinY = instance.UVMin.y + (hasBottom != 0 ? instance.Border.w : 0.0);
    float centerUVMaxY = instance.UVMax.y - (hasTop != 0 ? instance.Border.z : 0.0);

    float centerUVWidth = centerUVMaxX - centerUVMinX;
    float centerUVHeight = centerUVMaxY - centerUVMinY;

    // Calculate the original tile dimensions in world units.
    float tileWidth = centerUVWidth * instance.TextureSize.x / instance.PixelsPerUnit;
    float tileHeight = centerUVHeight * instance.TextureSize.y / instance.PixelsPerUnit;

    uint centerTilesX = tileWidth > 0.0
        ? max(1u, (uint)ceil(centerWidth / tileWidth))
        : 1u;

    uint centerTilesY = tileHeight > 0.0
        ? max(1u, (uint)ceil(centerHeight / tileHeight))
        : 1u;

    uint horizontalQuadCount = centerTilesX + hasLeft + hasRight;

    // Select the quad.
    uint quadID = vertexID / 6;
    uint quadX = quadID % horizontalQuadCount;
    uint quadY = quadID / horizontalQuadCount;

    float2 xPositionBounds;
    float2 xUVBounds;

    // Resolve horizontal region.
    if(hasLeft != 0 && quadX == 0)
    {
        xPositionBounds = float2(0.0, leftNorm);
        xUVBounds = float2(instance.UVMin.x, centerUVMinX);
    }
    else
    {
        uint centerTileX = quadX - hasLeft;

        if(centerTileX < centerTilesX)
        {
            float tileStart = centerTileX * tileWidth;
            float tileEnd = min(tileStart + tileWidth, centerWidth);

            xPositionBounds = float2(
                leftNorm + tileStart / instance.Size.x,
                leftNorm + tileEnd / instance.Size.x
            );

            float tileFraction = tileWidth > 0.0
                ? (tileEnd - tileStart) / tileWidth
                : 1.0;

            xUVBounds = float2(
                centerUVMinX,
                centerUVMinX + tileFraction * centerUVWidth
            );
        }
        else
        {
            xPositionBounds = float2(1.0 - rightNorm, 1.0);
            xUVBounds = float2(centerUVMaxX, instance.UVMax.x);
        }
    }

    float2 yPositionBounds;
    float2 yUVBounds;

    // Resolve vertical region, bottom to top.
    if(hasBottom != 0 && quadY == 0)
    {
        yPositionBounds = float2(0.0, bottomNorm);
        yUVBounds = float2(instance.UVMin.y, centerUVMinY);
    }
    else
    {
        uint centerTileY = quadY - hasBottom;

        if(centerTileY < centerTilesY)
        {
            float tileStart = centerTileY * tileHeight;
            float tileEnd = min(tileStart + tileHeight, centerHeight);

            yPositionBounds = float2(
                bottomNorm + tileStart / instance.Size.y,
                bottomNorm + tileEnd / instance.Size.y
            );

            float tileFraction = tileHeight > 0.0
                ? (tileEnd - tileStart) / tileHeight
                : 1.0;

            yUVBounds = float2(
                centerUVMinY,
                centerUVMinY + tileFraction * centerUVHeight
            );
        }
        else
        {
            yPositionBounds = float2(1.0 - topNorm, 1.0);
            yUVBounds = float2(centerUVMaxY, instance.UVMax.y);
        }
    }

    // Interpolate within the selected quad.
    float2 vertexCornerID = quadPositions[vertexID % 6];

    float2 vertexCoordinate = lerp(
        float2(xPositionBounds.x, yPositionBounds.x),
        float2(xPositionBounds.y, yPositionBounds.y),
        vertexCornerID
    );

    vertexPos = float3(
        (vertexCoordinate - instance.Pivot) * instance.Size,
        0.0
    );

    vertexUV = lerp(
        float2(xUVBounds.x, yUVBounds.x),
        float2(xUVBounds.y, yUVBounds.y),
        vertexCornerID
    );
}

[shader("vertex")]
VOut vsMain(int vertexID : SV_VertexID, SpriteInstanceInfo instance)
{
    VOut output;
    float4x4 model = instance.Model;

    int quadID = vertexID / 6;
    float2 vertexCornerID = quadPositions[vertexID % 6];

    float3 vertexPos = float3(0,0,0);
    if(instance.Tiled != 0)
        GetTiledVertexInfo(vertexID, instance, vertexPos, output.UV);
    else
        GetVertexInfo(vertexID, instance, vertexPos, output.UV);

    // float3 vertexPos = float3((vertexCornerID - instance.Pivot) * instance.Size, 0);

    output.FragPosition = mul(cameraData.viewProj, mul(model, float4(vertexPos, 1.0)));
    output.Position = mul(model, float4(vertexPos, 1.0)).xyz;

    // float3 rawNormal = float3(0,0,1);
    // float3x3 normalMatrix = transpose(adjugate(float3x3(model)));
    // output.Normal = normalize(mul(normalMatrix, rawNormal));
    output.Normal = float3(0,0,1);
    output.Color = float3(1,1,1);
    // output.UV = vertexCornerID;
    // output.UV = lerp(instance.UVMin, instance.UVMax, vertexCornerID);

    return output;
}

// Entry point
[shader("fragment")]
float4 fsMain(FIn input)
{
    float4 outColor = mainTex.Sample(input.UV);
    outColor = float4(outColor.rgb * materialInfo.color, outColor.a);
    if(materialInfo.doAlphaClipping != 0)
    {
        if(outColor.a < materialInfo.alphaClippingThreshold)
            discard;
        else
            outColor.w = 1;
    }
    return outColor;
    // return float4(input.UV.x, input.UV.y, 0, 1);
}