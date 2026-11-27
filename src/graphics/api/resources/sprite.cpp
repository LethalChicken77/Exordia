#include "sprite.hpp"
#include "graphics/graphics_data.hpp"


namespace graphics
{

void Sprite::SetTexture(TextureData* textureData)
{
    TextureHandle handle = textureData->graphicsHandle;
    if(!handle.IsValid())
    {
        Console::errorf("Attempt to set invalid texture handle on sprite {}", name, "Sprite");
        return;
    }
    texture = handle;
    TextureResource* textureP = graphicsData->textureRegistry.Get(texture);
    textureSize = textureData->GetDimensions();
    texturePPU = textureData->properties.pixelsPerUnit;
    Validate();
}

void Sprite::SetPosition(int x, int y)
{
    graphics::TextureResource* tex = graphicsData->textureRegistry.Get(texture);
    if(tex == nullptr)
    {
        Console::errorf("Sprite {} has invalid texture handle", name, "Sprite");
        return;
    }
    pos.x = x;
    pos.y = y;
    Validate();
}

void Sprite::SetSize(int w, int h)
{
    size.x = w;
    size.y = h;
    Validate();
}

void Sprite::SetLeft(int l) 
{ 
    left = l; 
    if(left > size.x - 1) 
        left = size.x - 1;
    if(left + right > size.x - 1)
        right = size.x - left - 1;
};
void Sprite::SetRight(int r)
{ 
    right = r; 
    if(right > size.x - 1) 
        right = size.x - 1;
    if(left + right > size.x - 1)
        left = size.x - right - 1;
};
void Sprite::SetTop(int t)
{ 
    top = t; 
    if(top > size.y - 1) 
        top = size.y - 1;
    if(top + bottom > size.y - 1)
        bottom = size.y - top - 1;
};
void Sprite::SetBottom(int b)
{ 
    bottom = b; 
    if(bottom > size.y - 1) 
        bottom = size.y - 1;
    if(bottom + bottom > size.y - 1)
        top = size.y - bottom - 1;
};

void Sprite::Validate()
{
    
}

uint32_t Sprite::Get9SliceQuadCount() const
{
    int horizontal = 1 + (left > 0) + (right > 0);
    int vertical = 1 + (top > 0) + (bottom > 0);

    return horizontal * vertical;
}

uint32_t Sprite::GetTiledQuadCount(glm::vec2 _size) const
{
    float leftSize = left / texturePPU;
    float rightSize = right / texturePPU;
    float topSize = top / texturePPU;
    float bottomSize = bottom / texturePPU;

    float centerWidth = (size.x - left - right) / texturePPU;
    float centerHeight = (size.y - top - bottom) / texturePPU;

    int horizontal = std::max(1, (int)std::ceil((_size.x - leftSize - rightSize) / centerWidth));
    int vertical = std::max(1, (int)std::ceil((_size.y - topSize - bottomSize) / centerHeight));

    horizontal += (left > 0) + (right > 0);
    vertical += (top > 0) + (bottom > 0);

    return horizontal * vertical;
}

glm::vec2 Sprite::GetSimpleSize() const
{
    return (glm::vec2)size / texturePPU;
}

} // namespace graphics