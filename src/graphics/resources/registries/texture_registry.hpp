#pragma once
#include "registry.hpp"
#include "graphics/api/resources/texture_data.hpp"
#include "graphics/resources/texture.hpp"

namespace graphics
{
    
class TextureRegistry : public GraphicsRegistry<Texture, TextureHandle>
{
public:
    void Init();
    TextureHandle Register(TextureData &texture);
    bool Update(TextureData &texture);
    using GraphicsRegistry<Texture, TextureHandle>::Deregister;
    inline bool Deregister(TextureData &texture)
    {
        return Deregister(texture.graphicsHandle);
    }

    Texture* Get(TextureHandle handle) const;
private:
    TextureHandle debugTextureHandle;
};

} // namespace graphics