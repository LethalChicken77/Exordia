#include "sprite.hpp"
#include "graphics/graphics_data.hpp"


namespace graphics
{

void Sprite::SetPosition(int x, int y)
{
    graphics::Texture* tex = graphicsData->textureRegistry.Get(texture);
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

} // namespace graphics